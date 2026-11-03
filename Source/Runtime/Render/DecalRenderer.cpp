#include "EnginePCH.h"

#include "DecalRenderer.h"
#include "Engine/DecalSceneProxy.h"
#include "Render/Material.h"
#include "RenderResourceManager.h"
#include "RenderCommand.h"

bool FDecalRenderer::Init()
{
	PipelineState.Shader = FRenderResourceManager::GetShaderProgram("Resources/Shader/DecalShader.hlsl");
	PipelineState.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	PipelineState.BlendState = EBlendState::AlphaBlend;

	ConstantBuffer = RenderCommand::CreateConstantBuffer(sizeof(FDecalConstants));

	return true;
}

void FDecalRenderer::Render(const TArray<FDecalSceneProxy*>& InDecalProxies, const FMatrix& ViewProjection, FTexture2D* InDepthTexture, FTexture2D* InObjectMaskTexture, uint32& OutDrawCount)
{
	FMatrix InvViewProjection = ViewProjection.Inverse();

	RenderCommand::BindPipelineState(PipelineState);
	RenderCommand::BindConstantBuffer(0, ConstantBuffer.get(), EShaderBindFlagBits::Pixel);
	RenderCommand::BindShaderResource(0, InDepthTexture, EShaderBindFlagBits::Pixel);
	RenderCommand::BindShaderResource(1, InObjectMaskTexture, EShaderBindFlagBits::Pixel);

	OutDrawCount = 0;
	for (FDecalSceneProxy* DecalProxy : InDecalProxies)
	{
		UMaterial* DecalMaterial = DecalProxy->GetDecalMaterial();
		if (!DecalMaterial)
		{
			continue;
		}

		FDecalConstants DecalConstants;
		DecalConstants.InvViewProjection = InvViewProjection;
		DecalConstants.InvWorld = DecalProxy->GetInvWorldMatrix();
		DecalConstants.Opacity = DecalProxy->GetOpacity();
		DecalConstants.FadeAlpha = DecalProxy->GetFadeAlpha();

		RenderCommand::UpdateBufferData(ConstantBuffer.get(), &DecalConstants);
		RenderCommand::BindShaderResource(2, DecalMaterial->Textures[0], EShaderBindFlagBits::Pixel);
		RenderCommand::BindSamplerState(0, DecalMaterial->SamplerState, EShaderBindFlagBits::Pixel);
		RenderCommand::Draw(3);

		OutDrawCount++;
	}

	RenderCommand::UnbindShaderResource(0, EShaderBindFlagBits::Pixel);
	RenderCommand::UnbindShaderResource(1, EShaderBindFlagBits::Pixel);
}
