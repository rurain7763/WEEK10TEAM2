#include "EnginePCH.h"
#include "Component/SkinnedMeshComponent.h"

#include "Animation/Skeleton.h"

USkinnedMeshComponent::~USkinnedMeshComponent() = default;

void USkinnedMeshComponent::PostLoad()
{
	Super::PostLoad();

	UpdateBoneMatrices();
}

void USkinnedMeshComponent::SetSkeletalMesh(USkeletalMesh* InMesh)
{
	SkeletalMesh = InMesh;

	ClearOverrideMaterials();

	if (SkeletalMesh)
	{
		const auto& Materials = SkeletalMesh->GetMaterials();
		for (int32 i = 0; i < Materials.Num(); i++)
		{
			SetMaterial(i, Materials[i]);
		}
	}

	UpdateBoneMatrices();
	MarkRenderStateDirty();
	OnTransformDirty();
}

void USkinnedMeshComponent::UpdateBoneMatrices()
{
	if (!SkeletalMesh)
	{
		return;
	}

	USkeleton* Skeleton = SkeletalMesh->GetSkeleton();
	if (!Skeleton)
	{
		return;
	}

	const TArray<FModelNode>& Nodes = Skeleton->GetNodes();
	const TArray<FModelBone>& Bones = Skeleton->GetBones();

	TArray<FMatrix> GlobalTransforms;
	GlobalTransforms.SetNum(Nodes.Num());
	for (int32 i = 0; i < Nodes.Num(); i++)
	{
		const FModelNode& Node = Nodes[i];

		FMatrix ParentMatrix = FMatrix::Identity;
		if (Node.ParentNodeIndex != -1)
		{
			ParentMatrix = GlobalTransforms[Node.ParentNodeIndex];
		}

		GlobalTransforms[i] = Node.LocalTransform * ParentMatrix;
	}

	BoneMatrices.SetNum(Bones.Num());
	for (int32 i = 0; i < Bones.Num(); i++)
	{
		const FModelBone& Bone = Bones[i];

		BoneMatrices[i] = Bone.InverseBindTransform * GlobalTransforms[Bone.NodeIndex];
	}

	MarkRenderStateDirty();
}

int32 USkinnedMeshComponent::GetNumMaterials() const
{
	return SkeletalMesh ? SkeletalMesh->GetMaterials().Num() : 0;
}

FString USkinnedMeshComponent::GetMaterialSlotName(int32 SlotIndex) const
{
	return SkeletalMesh ? SkeletalMesh->GetMeshData().MaterialSlots[SlotIndex].Name : FString();
}

UMaterial* USkinnedMeshComponent::GetDefaultMaterial(int32 SlotIndex) const
{
	return SkeletalMesh ? SkeletalMesh->GetMaterials()[SlotIndex] : nullptr;
}

const TArray<FMatrix>& USkinnedMeshComponent::GetBoneMatrices() const
{
	return BoneMatrices;
}

USkeletalMesh* USkinnedMeshComponent::GetSkeletalMesh()
{
	return SkeletalMesh;
}
