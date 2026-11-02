#include "EnginePCH.h"
#include "DecalRenderer.h"

#include "ColorWorkflow.h"
#include "Material.h"
#include "RenderCommand.h"
#include "RenderResourceManager.h"
#include "RenderingInfo.h"
#include "SceneRenderer.h"
#include "Core/EngineTimer.h"
#include "Math/Frustum.h"

#include <algorithm>

namespace {
	struct FDecalViewConstants {
		FMatrix mInverseViewProjection{};
		FVector4 mViewport{};
	};

	struct FDecalConstants {
		FMatrix mWorldToDecal{};
		FLinearColor mColor{};
		FVector2 mUVOffset{};
		uint32 mHasTexture{};
		float mPadding{};
	};

	static_assert(sizeof(FDecalViewConstants) == 80);
	static_assert(sizeof(FDecalConstants) == 96);

	float CalculateScreenFade(const FAABB& Bounds, const FSceneView& View, float FadeScreenSize) {
		if (FadeScreenSize <= 0.0f) {
			return 1.0f;
		}

		const float Radius{ Bounds.Extent.Length() };
		const FVector4 ClipCenter{ View.ViewProjection.TransformFVector4(FVector4{ Bounds.Center, 1.0f }) };
		if (!View.bOrthographic && ClipCenter.W <= Radius) {
			return 1.0f;
		}

		const FVector ProjectionX{ View.ViewProjection[0][0], View.ViewProjection[1][0], View.ViewProjection[2][0] };
		const FVector ProjectionY{ View.ViewProjection[0][1], View.ViewProjection[1][1], View.ViewProjection[2][1] };
		const float ProjectionScale{ std::max(ProjectionX.Length(), ProjectionY.Length()) };
		const float ScreenRadius{ Radius * ProjectionScale / std::max(std::abs(ClipCenter.W), SMALL_NUMBER) };
		return FMath::Clamp(ScreenRadius / FadeScreenSize - 1.0f, 0.0f, 1.0f);
	}
}

bool FDecalRenderer::Init() {
	mPipelineState.Shader = FRenderResourceManager::GetShaderProgram("Resources/Shader/DecalShader.hlsl");
	if (!mPipelineState.Shader || !mPipelineState.Shader->PixelShader->GetBindings().FindConstantBuffer("DecalViewConstants") || !mPipelineState.Shader->PixelShader->GetBindings().FindConstantBuffer("DecalConstants")) {
		HTR_LOG(Error, "[Decal] shader unavailable");
		return false;
	}

	mPipelineState.RasterizerState = ERasterizerState::SolidNone;
	mPipelineState.BlendState = EBlendState::AlphaBlend;
	mPipelineState.DepthStencilState = EDepthStencilState::Disabled;
	mViewConstants = RenderCommand::CreateConstantBuffer(sizeof(FDecalViewConstants));
	mDecalConstants = RenderCommand::CreateConstantBuffer(sizeof(FDecalConstants));
	return mViewConstants && mDecalConstants && mViewConstants->GetBuffer() && mDecalConstants->GetBuffer();
}

void FDecalRenderer::OnRender(FTexture2D* SceneDepth, const FSceneView& View, const FViewportSettings& Viewport, TArray<FDecalRenderData>& Decals) {
	if (!SceneDepth || Decals.IsEmpty() || !mViewConstants || !mDecalConstants || Viewport.Width == 0 || Viewport.Height == 0) {
		return;
	}

	FDecalViewConstants ViewConstants{};
	if (!View.ViewProjection.TryInverse(ViewConstants.mInverseViewProjection)) {
		return;
	}
	ViewConstants.mViewport = FVector4{ static_cast<float>(Viewport.StartX), static_cast<float>(Viewport.StartY), 1.0f / Viewport.Width, 1.0f / Viewport.Height };
	RenderCommand::UpdateBufferData(mViewConstants.get(), &ViewConstants);
	RenderCommand::BindPipelineState(mPipelineState);
	RenderCommand::BindConstantBuffer(0, mViewConstants.get(), EShaderBindFlagBits::Pixel);
	RenderCommand::BindConstantBuffer(1, mDecalConstants.get(), EShaderBindFlagBits::Pixel);
	RenderCommand::BindShaderResource(0, SceneDepth, EShaderBindFlagBits::Pixel, true);

	std::stable_sort(Decals.begin(), Decals.end(), [](const FDecalRenderData& Left, const FDecalRenderData& Right) {
		return Left.mSortOrder < Right.mSortOrder;
	});
	const FFrustumPlanes Frustum{ ExtractFrustumPlanes(View.ViewProjection) };
	const float Time{ EngineTimer::GetTotalTime() };

	for (const FDecalRenderData& Decal : Decals) {
		const UMaterial* Material{ Decal.mDecalMaterial };
		if (!Decal.mActive || !Material || Decal.mOpacity <= 0.0f) {
			continue;
		}

		const FAABB Bounds{ MakeWorldBounds(FBox{ -Decal.mDecalSize, Decal.mDecalSize }.GetWorldAABB(Decal.mWorldMatrix)) };
		if (!IsAABBInFrustum(Bounds, Frustum)) {
			continue;
		}

		const float Fade{ CalculateScreenFade(Bounds, View, Decal.mFadeScreenSize) };
		const FLinearColor Color{ Decal.mDecalColor.R * Material->BaseColor.R, Decal.mDecalColor.G * Material->BaseColor.G, Decal.mDecalColor.B * Material->BaseColor.B, FMath::Clamp(Decal.mDecalColor.A * Material->BaseColor.A * Decal.mOpacity * Fade, 0.0f, 1.0f) };
		if (Color.A <= 0.0f) {
			continue;
		}

		UTexture2D* Texture{ Material->Textures.IsEmpty() ? nullptr : Material->Textures[0] };
		FDecalConstants Constants{};
		Constants.mWorldToDecal = Decal.mWorldToDecalMatrix;
		Constants.mColor = ColorWorkflow::ToShaderColor(Color);
		Constants.mUVOffset = Material->UVScrollSpeed * Time;
		Constants.mHasTexture = Texture && Texture->GetResource() ? 1u : 0u;
		RenderCommand::UpdateBufferData(mDecalConstants.get(), &Constants);
		RenderCommand::BindShaderResource(1, Texture, EShaderBindFlagBits::Pixel);
		RenderCommand::BindSamplerState(0, Material->SamplerState, EShaderBindFlagBits::Pixel);
		RenderCommand::Draw(3);
	}

	RenderCommand::UnbindShaderResource(0, EShaderBindFlagBits::Pixel);
	RenderCommand::UnbindShaderResource(1, EShaderBindFlagBits::Pixel);
}
