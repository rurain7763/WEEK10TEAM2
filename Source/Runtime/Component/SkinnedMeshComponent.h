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

	void SetSkeletalMesh(USkeletalMesh* InMesh);

	void UpdateBoneMatrices();

	virtual int32 GetNumMaterials() const override;

	virtual FString GetMaterialSlotName(int32 SlotIndex) const override;

	virtual UMaterial* GetDefaultMaterial(int32 SlotIndex) const override;

	const TArray<FMatrix>& GetBoneMatrices() const;

	USkeletalMesh* GetSkeletalMesh();

protected:
	USkeletalMesh* SkeletalMesh = nullptr;

	TArray<FMatrix> BoneMatrices;
};
