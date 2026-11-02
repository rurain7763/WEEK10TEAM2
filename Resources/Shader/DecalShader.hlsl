#pragma pack_matrix(row_major)

#define OM_RECIEVEDECALS 0x4

cbuffer DecalConstants : register(b0)
{
    matrix InvViewProjection;
    matrix InvWorld;
    float4 Tint;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

Texture2D<float> DepthTexture : register(t0);
Texture2D<uint> ObjectMaskTexture : register(t1);
Texture2D DecalTexture : register(t2);

SamplerState DecalSampler : register(s0);

PS_INPUT mainVS(uint VertexID : SV_VertexID)
{
	PS_INPUT Output;
    
    float2 UV = float2((VertexID << 1) & 2, VertexID & 2);

    Output.Position = float4(UV * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    Output.UV = UV;
    
	return Output;
}

float4 mainPS(PS_INPUT Input) : SV_TARGET
{
    float Depth = DepthTexture.Load(int3(Input.Position.xy, 0));
    uint ObjectMask = ObjectMaskTexture.Load(int3(Input.Position.xy, 0));
    
    if ((ObjectMask & OM_RECIEVEDECALS) != OM_RECIEVEDECALS)
    {
        discard;
    }
    
    if (Depth >= 1.0f)
    {
        discard;
    }
    
    float2 NDCXY = Input.UV * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
    float4 NDC = float4(NDCXY, Depth, 1.0f);
    float4 WorldPosition = mul(NDC, InvViewProjection);
    WorldPosition /= WorldPosition.w;
    
    float4 DecalPosition = mul(WorldPosition, InvWorld);
    if (DecalPosition.x < -0.5f || DecalPosition.x > 0.5f || DecalPosition.y < -0.5f || DecalPosition.y > 0.5f || DecalPosition.z < -0.5f || DecalPosition.z > 0.5f)
    {
        discard;
    }
    
    float2 DecalUV = DecalPosition.yz + float2(0.5, 0.5);
    DecalUV.y = 1.0 - DecalUV.y;
    
    float4 FinalColor = DecalTexture.Sample(DecalSampler, DecalUV);
    FinalColor *= Tint;
    
    return FinalColor;
}
