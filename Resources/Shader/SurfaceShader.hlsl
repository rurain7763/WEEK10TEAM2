#pragma pack_matrix(row_major)
        
struct FVSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Tangent : TANGENT;
    float4 Color : COLOR; // 지금은 쓰지 않지만 나중에 쓰게 될 수 있음
    float2 UV : TEXCOORD;
#if USE_SKINNING
    int4 BoneIndices : BLENDINDICES;
    float4 BoneWeights : BLENDWEIGHT;
    int BoneInfluenceCount : BONECOUNT;
#endif
};

struct FPSInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

struct FPSOutput
{
    uint ObjectMask : SV_Target0;
};

cbuffer ViewConstants : register(b0)
{
    matrix ViewProjection;
    float3 CameraPosition;
    float ViewPadding;
};

cbuffer PerObjectConstants : register(b1)
{
    matrix World;
    matrix NormalMatrix; // World의 역전치
    uint ObjectMask;
    uint3 PerObjectConstantsPadding;
};

#if USE_SKINNING
StructuredBuffer<float4x4> BoneMatrices : register(t0);
#endif

FPSInput mainVS(FVSInput Input)
{
    FPSInput Output;

#if USE_SKINNING
    float3 SkinnedPosition = float3(0, 0, 0);
    if (Input.BoneInfluenceCount == 0)
    {
        SkinnedPosition = Input.Position;
    }
    
    for (int i = 0; i < Input.BoneInfluenceCount; i++)
    {
        int BoneIndex = Input.BoneIndices[i];
        float BoneWeight = Input.BoneWeights[i];
        float4 Position = mul(float4(Input.Position, 1.0), BoneMatrices[BoneIndex]) * BoneWeight;
        SkinnedPosition += Position.xyz;
    }
    
    float4 WorldPosition = mul(float4(SkinnedPosition, 1.0f), World);
#else
    float4 WorldPosition = mul(float4(Input.Position, 1.0f), World);
#endif

    Output.Position = mul(WorldPosition, ViewProjection);
    Output.UV = Input.UV;

    return Output;
}

FPSOutput mainPS(FPSInput Input)
{
    FPSOutput Output;
    
    Output.ObjectMask = ObjectMask;
    
    return Output;
}
