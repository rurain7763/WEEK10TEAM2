#include "EnginePCH.h"
#include "Component/SkinnedMeshComponent.h"

#include "Animation/Skeleton.h"

USkinnedMeshComponent::~USkinnedMeshComponent() = default;

void USkinnedMeshComponent::PostLoad()
{
	Super::PostLoad();

	UpdateBoneMatrices();
}

void USkinnedMeshComponent::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);
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

	ResetPose();
	UpdateBoneMatrices();
	MarkRenderStateDirty();
	OnTransformDirty();
}

void USkinnedMeshComponent::ResetPose()
{
	LocalPose.Reset();
	USkeleton* Skeleton = SkeletalMesh ? SkeletalMesh->GetSkeleton() : nullptr;
	if (!Skeleton)
		return;

	const TArray<FModelNode>& Nodes = Skeleton->GetNodes();
	LocalPose.SetNum(Nodes.Num());
	for (int32 i = 0; i < Nodes.Num(); i++)
		LocalPose[i] = Nodes[i].LocalTransform;
}

void USkinnedMeshComponent::SetBoneComponentMatrix(const int32 BoneIndex, const FMatrix& ComponentMatrix)
{
	USkeleton* Skeleton = SkeletalMesh ? SkeletalMesh->GetSkeleton() : nullptr;
	if (!Skeleton)
		return;
	const TArray<FModelBone>& Bones = Skeleton->GetBones();
	const TArray<FModelNode>& Nodes = Skeleton->GetNodes();
	if (BoneIndex < 0 || BoneIndex >= Bones.Num() || LocalPose.Num() != Nodes.Num())
		return;

	// 행 벡터 규약: Global = Local × Global(부모) 이므로 Local = Global × Global(부모)⁻¹
	const int32 NodeIndex = Bones[BoneIndex].NodeIndex;
	const int32 ParentNodeIndex = Nodes[NodeIndex].ParentNodeIndex;
	const FMatrix ParentGlobal = ParentNodeIndex != -1 ? NodeGlobalTransforms[ParentNodeIndex] : FMatrix::Identity;
	LocalPose[NodeIndex] = ComponentMatrix * ParentGlobal.Inverse();

	// 자식 본도 이 본을 따라가도록 계층 전체를 다시 누적한다
	UpdateBoneMatrices();
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

	// 로드 직후처럼 포즈가 아직 없으면 에셋 기본 자세로 시작한다
	if (LocalPose.Num() != Nodes.Num())
		ResetPose();

	NodeGlobalTransforms.SetNum(Nodes.Num());          // 지역 GlobalTransforms 대신 멤버 사용
	for (int32 i = 0; i < Nodes.Num(); i++)
	{
		const FModelNode& Node = Nodes[i];
		FMatrix ParentMatrix = FMatrix::Identity;
		if (Node.ParentNodeIndex != -1)
			ParentMatrix = NodeGlobalTransforms[Node.ParentNodeIndex];
		NodeGlobalTransforms[i] = LocalPose[i] * ParentMatrix;
	}

	BoneMatrices.SetNum(Bones.Num());
	for (int32 i = 0; i < Bones.Num(); i++)
	{
		const FModelBone& Bone = Bones[i];

		BoneMatrices[i] = Bone.InverseBindTransform * NodeGlobalTransforms[Bone.NodeIndex];
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
FMatrix USkinnedMeshComponent::GetBoneComponentMatrix(const int32 BoneIndex) const
{
	USkeleton* Skeleton = SkeletalMesh ? SkeletalMesh->GetSkeleton() : nullptr;
	if (!Skeleton) return FMatrix::Identity;
	const TArray<FModelBone>& Bones = Skeleton->GetBones();
	if (BoneIndex < 0 || BoneIndex >= Bones.Num()) return FMatrix::Identity;
	return NodeGlobalTransforms[Bones[BoneIndex].NodeIndex];
}