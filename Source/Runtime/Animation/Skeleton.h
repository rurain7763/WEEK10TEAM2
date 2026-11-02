#pragma once

#include "Asset/Asset.h"
#include "Asset/ModelData.h"

class USkeleton : public UAsset
{
	DECLARE_CLASS(USkeleton, UAsset)

public:
	void Initialize(const TArray<FModelNode>& InNodes, const TArray<FModelBone>& InBones);

	// 배열을 복사하지 않고 빌려준다. 스키닝·본 트리가 매 프레임 읽는다
	const TArray<FModelNode>& GetNodes() const;

	const TArray<FModelBone>& GetBones() const;

private:
	TArray<FModelNode> Nodes;
	TArray<FModelBone> Bones;
};
