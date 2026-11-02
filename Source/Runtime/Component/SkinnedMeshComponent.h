#pragma once

#include "Component/MeshComponent.h"
#include "Render/SkeletalMesh.h"

class USkinnedMeshComponent : public UMeshComponent
{
	DECLARE_CLASS(USkinnedMeshComponent, UMeshComponent)

	REFLECT_START(USkinnedMeshComponent)
		PROPERTY(SkeletalMesh)
	REFLECT_END()

public:
	virtual ~USkinnedMeshComponent();

	virtual void PostLoad() override;

	virtual void PostDuplicate(bool bDuplicateForPIE) override;

	void SetSkeletalMesh(USkeletalMesh* InMesh);

	void UpdateBoneMatrices();

	virtual int32 GetNumMaterials() const override;

	virtual FString GetMaterialSlotName(int32 SlotIndex) const override;

	virtual UMaterial* GetDefaultMaterial(int32 SlotIndex) const override;

	const TArray<FMatrix>& GetBoneMatrices() const;

	USkeletalMesh* GetSkeletalMesh();

	FMatrix GetBoneComponentMatrix(const int32 BoneIndex) const;
	// 본의 컴포넌트 기준 행렬을 지정한다. 부모 기준 로컬로 바꿔 이 컴포넌트의 포즈에 넣고 스키닝 행렬을 다시 만든다.
	// 에셋(USkeleton)은 바꾸지 않으므로 같은 메시를 쓰는 다른 컴포넌트는 움직이지 않는다
	void SetBoneComponentMatrix(int32 BoneIndex, const FMatrix& ComponentMatrix);
	// 포즈를 에셋의 기본 자세로 되돌린다
	void ResetPose();

protected:
	USkeletalMesh* SkeletalMesh = nullptr;

	// [노드] = 부모 기준 로컬 트랜스폼. 이 컴포넌트만의 포즈이며 처음에는 에셋 노드의 LocalTransform을 복사한다
	TArray<FMatrix> LocalPose;
	TArray<FMatrix> BoneMatrices;
	TArray<FMatrix> NodeGlobalTransforms;
};
