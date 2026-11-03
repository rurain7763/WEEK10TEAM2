// 기존 표면 셰이더의 조명 및 그림자 함수 재사용
#define UBER_LIT_LIBRARY_ONLY
#define VIEWMODE 0
#define LIGHTING_MODEL 2
#define SHADING_MODEL 1
#define HAS_NORMAL_MAP 1
#define USE_SKINNING 0
#define RECEIVE_SHADOW 1
#include "UberLit.hlsl"

// 카메라 깊이와 불투명 표면 정보
Texture2D<float> SceneDepth : register(t4);
Texture2D<float4> NormalReceiverTexture : register(t5);

// 화면 위치 복원과 데칼 배치의 전용 상수
cbuffer DecalConstants : register(b7)
{
    matrix InverseViewProjection;
    matrix WorldToDecal;
    matrix DecalToWorld;
    float4 DecalColor;
    float2 ViewportOrigin;
    float2 ViewportSize;
    uint bUnlit;
    float3 DecalPadding;
};
struct FDecalPSInput
{
    float4 Position : SV_POSITION;
};

// 단위 박스 위치의 정점 입력
struct FDecalVSInput
{
    float3 Position : POSITION;
};

// 뷰포트 좌표와 D3D 깊이를 이용한 월드 위치 복원
float3 ReconstructWorldPosition(float2 PixelPosition, float Depth)
{
    float2 LocalPixel = PixelPosition - ViewportOrigin;
    float2 ScreenUV = LocalPixel / ViewportSize;
    float2 NDC = ScreenUV * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
    float4 WorldPositionH = mul(float4(NDC, Depth, 1.0f), InverseViewProjection);
    return WorldPositionH.xyz / WorldPositionH.w;
}

// 현재 뷰포트 내부의 유효 깊이 복원
bool TryLoadWorldPosition(float2 PixelPosition, out float3 WorldPosition)
{
    WorldPosition = 0.0f;
    if (any(PixelPosition < ViewportOrigin) || any(PixelPosition >= ViewportOrigin + ViewportSize))
    {
        return false;
    }

    float Depth = SceneDepth.Load(int3(int2(PixelPosition), 0));
    if (Depth >= 1.0f)
    {
        return false;
    }

    WorldPosition = ReconstructWorldPosition(PixelPosition, Depth);
    return true;
}

// 중심 접평면 기준의 한 픽셀 위치 변화량
float3 GetPlaneDerivative(float2 PixelPosition, float2 Axis, float3 WorldPosition, float3 GeoNormal)
{
    float2 NeighborPixel = PixelPosition + Axis;
    float3 RayOrigin = ReconstructWorldPosition(NeighborPixel, 0.0f);
    float3 RayDirection = ReconstructWorldPosition(NeighborPixel, 0.5f) - RayOrigin;
    float Denominator = dot(RayDirection, GeoNormal);
    if (Denominator * Denominator <= 1.0e-8f * dot(RayDirection, RayDirection))
    {
        return 0.0f;
    }

    float RayDistance = dot(WorldPosition - RayOrigin, GeoNormal) / Denominator;
    return RayOrigin + RayDirection * RayDistance - WorldPosition;
}

// 깊이 불연속을 제외한 화면 양의 방향 미분
float3 GetSurfaceDerivative(float2 PixelPosition, float2 Axis, float3 WorldPosition, float3 GeoNormal)
{
    float3 ExpectedDerivative = GetPlaneDerivative(PixelPosition, Axis, WorldPosition, GeoNormal);
    float3 Derivative = ExpectedDerivative;
    float BestError = dot(ExpectedDerivative, ExpectedDerivative);
    if (BestError == 0.0f)
    {
        return Derivative;
    }

    // 한 픽셀 예상 변화량을 기준으로 한 이웃 선택
    [unroll]
    for (int Side = -1; Side <= 1; Side += 2)
    {
        float3 NeighborPosition;
        if (!TryLoadWorldPosition(PixelPosition + Axis * Side, NeighborPosition))
        {
            continue;
        }

        float3 Candidate = (NeighborPosition - WorldPosition) * Side;
        float3 Error = Candidate - ExpectedDerivative;
        float ErrorSquared = dot(Error, Error);
        if (ErrorSquared < BestError)
        {
            Derivative = Candidate;
            BestError = ErrorSquared;
        }
    }
    return Derivative;
}

// 투영 UV 미분 기준의 노멀맵 월드 변환
float3 EvaluateDecalNormal(float2 UV, float2 UVdx, float2 UVdy, float3 PositionDx, float3 PositionDy, float3 GeoNormal)
{
    float Determinant = UVdx.x * UVdy.y - UVdx.y * UVdy.x;
    float DerivativeScale = dot(UVdx, UVdx) * dot(UVdy, UVdy);
    if (Determinant * Determinant <= 1.0e-6f * DerivativeScale)
    {
        return GeoNormal;
    }

    float3 TangentRaw = (PositionDx * UVdy.y - PositionDy * UVdx.y) / Determinant;
    float3 BitangentRaw = (PositionDy * UVdx.x - PositionDx * UVdy.x) / Determinant;
    float3 Tangent = TangentRaw - GeoNormal * dot(TangentRaw, GeoNormal);
    float TangentLengthSquared = dot(Tangent, Tangent);
    if (TangentLengthSquared <= 1.0e-8f * dot(TangentRaw, TangentRaw))
    {
        return GeoNormal;
    }

    // 직교화 및 반전 UV의 B축 방향 유지
    Tangent *= rsqrt(TangentLengthSquared);
    float3 Bitangent = cross(GeoNormal, Tangent);
    if (dot(Bitangent, BitangentRaw) < 0.0f)
    {
        Bitangent = -Bitangent;
    }

    // 원시 노멀 데이터 복원 및 기존 Green 반전 적용
    float3 TextureNormal = NormalTexture.SampleGrad(AlbedoSampler, UV, UVdx, UVdy).xyz * 2.0f - 1.0f;
    TextureNormal.y = bFlipNormalGreen != 0u ? -TextureNormal.y : TextureNormal.y;
    float3x3 TBN = float3x3(Tangent, Bitangent, GeoNormal);
    return normalize(mul(TextureNormal, TBN));
}

// 데칼 볼륨의 화면 투영
FDecalPSInput mainVS(FDecalVSInput Input)
{
    FDecalPSInput Output;
    float4 WorldPosition = mul(float4(Input.Position, 1.0f), DecalToWorld);
    Output.Position = mul(WorldPosition, ViewProjection);
    return Output;
}

// 화면 깊이 기반 데칼 투영 및 자체 조명
float4 mainPS(FDecalPSInput Input) : SV_TARGET
{
    int2 PixelCoord = int2(Input.Position.xy);
    float4 Receiver = NormalReceiverTexture.Load(int3(PixelCoord, 0)); // 리시버의 월드 노멀과 데칼 리시브 여부
    float Depth = SceneDepth.Load(int3(PixelCoord, 0));

    // 리시버의 알파값<0.5 는 ReceiveDecals가 False. Depth>=1은 배경.
    if (Receiver.a < 0.5f || Depth >= 1.0f)
    {
        discard;
    }

    float3 WorldPosition = ReconstructWorldPosition(Input.Position.xy, Depth);

    // 중심 원점의 단위 데칼 볼륨 내부 제한
    float3 DecalPosition = mul(float4(WorldPosition, 1.0f), WorldToDecal).xyz;
    clip(0.5f - abs(DecalPosition));

    // 표면 미분과 노멀맵 변환의 기하 노멀
    float3 GeoNormal = normalize(Receiver.xyz);

    // +X 투영과 YZ 평면의 텍스처 좌표 생성
    float2 DecalUV = float2(DecalPosition.y + 0.5f, 0.5f - DecalPosition.z);

    // 동일한 표면 미분을 이용한 색상 및 노멀 샘플링
    float3 PositionDx = GetSurfaceDerivative(Input.Position.xy, float2(1.0f, 0.0f), WorldPosition, GeoNormal);
    float3 PositionDy = GetSurfaceDerivative(Input.Position.xy, float2(0.0f, 1.0f), WorldPosition, GeoNormal);
    float3 DecalDx = mul(float4(PositionDx, 0.0f), WorldToDecal).xyz;
    float3 DecalDy = mul(float4(PositionDy, 0.0f), WorldToDecal).xyz;
    float2 UVdx = float2(DecalDx.y, -DecalDx.z);
    float2 UVdy = float2(DecalDy.y, -DecalDy.z);
    float2 SampleUV = DecalUV + UVScrollOffset;

    float4 SurfaceColor = AlbedoTexture.SampleGrad(AlbedoSampler, SampleUV, UVdx, UVdy) * BaseColor * DecalColor;
    float Alpha = saturate(SurfaceColor.a);

    if (Alpha <= 0.0f)
    {
        discard;
    }

    // Unlit 뷰의 색상 출력
    if (bUnlit != 0u)
    {
        return float4(SurfaceColor.rgb, Alpha);
    }

    // 수신 표면의 기하 노멀과 기존 광원·그림자 계산
    float3 N = EvaluateDecalNormal(SampleUV, UVdx, UVdy, PositionDx, PositionDy, GeoNormal);
    float3 V = normalize(CameraPosition - WorldPosition);
    float2 LocalPixel = Input.Position.xy - ViewportOrigin;
    float3 DiffuseLight;
    float3 SpecularLight;

    ComputeTiledLighting(N, GeoNormal, WorldPosition, V, Shininess, LocalPixel, DiffuseLight, SpecularLight);

    float3 LitColor = SurfaceColor.rgb * (AmbientLight.Color + DiffuseLight) + SpecularColor * SpecularLight;

    return float4(LitColor, Alpha);
}