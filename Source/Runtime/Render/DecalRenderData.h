#pragma once

#include "Math/Color.h"
#include "Math/Matrix.h"
#include "Math/Vector.h"

class UMaterial;

struct FDecalRenderData {
	FMatrix mWorldMatrix{};
	FMatrix mWorldToDecalMatrix{};
	FVector mDecalSize{};
	UMaterial* mDecalMaterial{};
	FLinearColor mDecalColor{};
	float mOpacity{};
	int32 mSortOrder{};
	float mFadeScreenSize{};
	bool mActive{};
};
