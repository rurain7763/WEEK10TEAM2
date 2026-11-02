#include "EnginePCH.h"
#include "Animation/Skeleton.h"

void USkeleton::Initialize(const TArray<FModelNode>& InNodes, const TArray<FModelBone>& InBones)
{
	Nodes = InNodes;
	Bones = InBones;
}

const TArray<FModelNode>& USkeleton::GetNodes() const
{
	return Nodes;
}

const TArray<FModelBone>& USkeleton::GetBones() const
{
	return Bones;
}
