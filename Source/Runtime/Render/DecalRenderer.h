#pragma once

#include "Buffer.h"
#include "PipelineState.h"
#include "Containers/Array.h"
#include "Math/Matrix.h"

class FDecalSceneProxy;
class FTexture2D;

class FDecalRenderer
{
public:
	bool Init();

	void Render(const TArray<FDecalSceneProxy*>& InDecalProxies, const FMatrix& ViewProjection, FTexture2D* InDepthTexture, FTexture2D* InObjectMaskTexture);

private:
	struct FDecalConstants
	{
		FMatrix InvViewProjection;
		FMatrix InvWorld;
		float Opacity;
		float Padding[3];
	};

	FPipelineState PipelineState;
	TUniquePtr<FConstantBuffer> ConstantBuffer;
};
