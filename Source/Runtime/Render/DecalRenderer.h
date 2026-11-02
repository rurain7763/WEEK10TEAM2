#pragma once

#include "Buffer.h"
#include "PipelineState.h"
#include "Containers/Array.h"
#include "Math/Matrix.h"
#include "Math/Color.h"

class FDecalSceneProxy;
class FTexture2D;

// 데칼 투영의 GPU 자원 및 드로우 제출 관리
class FDecalRenderer
{
public:
	bool Init();

	void Render(const TArray<FDecalSceneProxy*>& InDecalProxies, const FMatrix& ViewProjection,
		FTexture2D* InDepthTexture, FTexture2D* InObjectMaskTexture, uint32& OutDrawCount, bool bGameView);

private:
	// 투영 변환 및 페이드가 반영된 데칼 색상
	struct FDecalConstants
	{
		FMatrix InvViewProjection;
		FMatrix InvWorld;
		FLinearColor Tint;
	};
	static_assert(sizeof(FDecalConstants) == 144);

	FPipelineState PipelineState;
	TUniquePtr<FConstantBuffer> ConstantBuffer;

	// 뷰별 가시 데칼 목록의 메모리 재사용
	TArray<const FDecalSceneProxy*> VisibleDecals;
};
