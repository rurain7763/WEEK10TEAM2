#include "EnginePCH.h"

#include "DecalRenderer.h"
#include "Asset/AssetManager.h"
#include "Component/DecalComponent.h"
#include "Engine/DecalSceneProxy.h"
#include "Math/Frustum.h"
#include "Render/ColorWorkflow.h"
#include "Render/Material.h"
#include "RenderResourceManager.h"
#include "RenderCommand.h"
#include <algorithm>

bool FDecalRenderer::Init()
{
	PipelineState.Shader = FRenderResourceManager::GetShaderProgram("Resources/Shader/DecalShader.hlsl");
	PipelineState.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	PipelineState.BlendState = EBlendState::AlphaBlend;
	PipelineState.DepthStencilState = EDepthStencilState::Disabled;

	ConstantBuffer = RenderCommand::CreateConstantBuffer(sizeof(FDecalConstants));

	return true;
}

// 가시 데칼의 선별 및 DBuffer 제출 횟수 집계
void FDecalRenderer::Render(const TArray<FDecalSceneProxy*>& InDecalProxies, const FMatrix& ViewProjection,
	FTexture2D* InDepthTexture, FTexture2D* InObjectMaskTexture, uint32& OutDrawCount, bool bGameView)
{
	OutDrawCount = 0;

	// 현재 뷰에 영향을 주는 데칼 선별
	VisibleDecals.Reset();
	const FFrustumPlanes Frustum = ExtractFrustumPlanes(ViewProjection);
	for (const FDecalSceneProxy* Proxy : InDecalProxies)
	{
		if (!Proxy->IsVisible() || (bGameView && Proxy->GetComponent()->IsHiddenInGame())
			|| !IsAABBInFrustum(MakeWorldBounds(Proxy->GetBounds()), Frustum))
		{
			continue;
		}
		VisibleDecals.Add(Proxy);
	}
	if (VisibleDecals.IsEmpty())
	{
		return;
	}

	// 합성 순서와 동일 순위의 결정적 정렬
	std::sort(VisibleDecals.begin(), VisibleDecals.end(),
		[](const FDecalSceneProxy* Left, const FDecalSceneProxy* Right)
		{
			if (Left->GetSortOrder() != Right->GetSortOrder())
			{
				return Left->GetSortOrder() < Right->GetSortOrder();
			}
			return Left->GetId() < Right->GetId();
		});

	FDecalConstants DecalConstants{};
	DecalConstants.InvViewProjection = ViewProjection.Inverse();
	UTexture2D* WhiteTexture = UAssetManager::GetAssetByPath<UTexture2D>("WhiteTexture");

	RenderCommand::BindPipelineState(PipelineState);
	RenderCommand::BindConstantBuffer(0, ConstantBuffer.get(), EShaderBindFlagBits::Pixel);
	RenderCommand::BindShaderResource(0, InDepthTexture, EShaderBindFlagBits::Pixel);
	RenderCommand::BindShaderResource(1, InObjectMaskTexture, EShaderBindFlagBits::Pixel);

	for (const FDecalSceneProxy* DecalProxy : VisibleDecals)
	{
		UMaterial* DecalMaterial = DecalProxy->GetMaterial();

		// 데칼·머티리얼 색상과 페이드의 단일 결합
		const FLinearColor DecalColor = ColorWorkflow::ToShaderColor(DecalProxy->GetColor());
		const FLinearColor BaseColor = ColorWorkflow::ToShaderColor(DecalMaterial->BaseColor);
		DecalConstants.InvWorld = DecalProxy->GetWorldToDecal();
		DecalConstants.Tint = FLinearColor(DecalColor.R * BaseColor.R, DecalColor.G * BaseColor.G,
			DecalColor.B * BaseColor.B, DecalColor.A * BaseColor.A * DecalProxy->GetFadeAlpha());
		RenderCommand::UpdateBufferData(ConstantBuffer.get(), &DecalConstants);

		// 미지정 Albedo의 기존 흰색 텍스처 대체
		UTexture2D* Albedo = DecalMaterial->Textures.Num() > 0 && DecalMaterial->Textures[0]
			? DecalMaterial->Textures[0] : WhiteTexture;
		RenderCommand::BindShaderResource(2, Albedo, EShaderBindFlagBits::Pixel);
		RenderCommand::BindSamplerState(0, DecalMaterial->SamplerState, EShaderBindFlagBits::Pixel);
		RenderCommand::Draw(3);
		++OutDrawCount;
	}

	// 후속 패스의 입력·출력 자원 충돌 방지
	for (uint32 Slot = 0; Slot <= 2; ++Slot)
	{
		RenderCommand::UnbindShaderResource(Slot, EShaderBindFlagBits::Pixel);
	}
	RenderCommand::SetBlendState(EBlendState::Opaque);
	RenderCommand::SetDepthStencilState(EDepthStencilState::Default);
}
