#pragma once

#include "Buffer.h"
#include "DecalRenderData.h"
#include "PipelineState.h"

class FTexture2D;
struct FSceneView;
struct FViewportSettings;

class FDecalRenderer final {
public:
	FDecalRenderer() = default;
	~FDecalRenderer() = default;

public:
	bool Init();
	void OnRender(FTexture2D* SceneDepth, const FSceneView& View, const FViewportSettings& Viewport, TArray<FDecalRenderData>& Decals);

private:
	FPipelineState mPipelineState{};
	TUniquePtr<FConstantBuffer> mViewConstants{};
	TUniquePtr<FConstantBuffer> mDecalConstants{};
};
