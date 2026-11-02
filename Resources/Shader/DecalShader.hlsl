#pragma pack_matrix(row_major)

cbuffer DecalViewConstants : register(b0) {
	float4x4 InverseViewProjection;
	float4 Viewport;
};

cbuffer DecalConstants : register(b1) {
	float4x4 WorldToDecal;
	float4 DecalColor;
	float2 UVOffset;
	uint HasTexture;
	float Padding;
};

Texture2D<float> SceneDepth : register(t0);
Texture2D<float4> DecalTexture : register(t1);
SamplerState DecalSampler : register(s0);

struct FDecalPixelInput {
	float4 mPosition : SV_POSITION;
};

FDecalPixelInput mainVS(uint VertexID : SV_VertexID) {
	FDecalPixelInput Output = { float4(0.0f, 0.0f, 0.0f, 0.0f) };
	float2 UV = { float2((VertexID << 1) & 2, VertexID & 2) };
	Output.mPosition = float4(UV * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
	return Output;
}

bool IsInsideDecal(float3 DecalPosition) {
	return all(abs(DecalPosition) <= 1.0f);
}

float4 mainPS(FDecalPixelInput Input) : SV_Target {
	float Depth = { SceneDepth.Load(int3(Input.mPosition.xy, 0)) };
	if (Depth >= 1.0f) {
		discard;
	}

	float2 ScreenUV = { (Input.mPosition.xy - Viewport.xy) * Viewport.zw };
	float2 NDC = { ScreenUV * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f) };
	float4 WorldPosition = { mul(float4(NDC, Depth, 1.0f), InverseViewProjection) };
	WorldPosition /= WorldPosition.w;
	float3 DecalPosition = { mul(float4(WorldPosition.xyz, 1.0f), WorldToDecal).xyz };
	if (!IsInsideDecal(DecalPosition)) {
		discard;
	}

	float2 UV = { DecalPosition.yz * float2(0.5f, -0.5f) + 0.5f + UVOffset };
	float4 Color = { DecalColor };
	if (HasTexture != 0) {
		Color *= DecalTexture.Sample(DecalSampler, UV);
	}
	return Color;
}
