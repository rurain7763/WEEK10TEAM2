#pragma once

#include "Asset/Asset.h"
#include "Asset/ModelData.h"

class USkeleton : public UAsset
{
	DECLARE_CLASS(USkeleton, UAsset)

public:
	void Initialize(const TArray<FModelNode>& InNodes, const TArray<FModelBone>& InBones);

	const TArray<FModelNode> GetNodes() const;

	const TArray<FModelBone> GetBones() const;

private:
	TArray<FModelNode> Nodes;
	TArray<FModelBone> Bones;
};
