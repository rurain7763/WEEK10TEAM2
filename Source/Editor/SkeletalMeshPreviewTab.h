#pragma once
#include "Editor/FEditorTab.h"
#include "Render/SkeletalMesh.h"

#include <functional>
#include "PreviewScene.h"
#include "GameFramework/Actor/SkeletalMeshActor.h"

class FViewportsPanel;

class FSkeletalMeshPreviewTab : public FEditorTab
{
public:
	explicit FSkeletalMeshPreviewTab(USkeletalMesh* InSkeletalMesh);
	virtual ~FSkeletalMeshPreviewTab() = default;;
	FSkeletalMeshPreviewTab(const FSkeletalMeshPreviewTab&) = delete;
	FSkeletalMeshPreviewTab& operator=(const FSkeletalMeshPreviewTab&) = delete;

	virtual const UObject* GetEditingAsset() const { return SkeletalMesh; };
private:
	FViewportsPanel* ViewportPanel = nullptr;
	const FGizmo* Gizmo = nullptr;
	USkeletalMesh* SkeletalMesh = nullptr;
	//프리뷰 씬을 담는 PreviewScene. FSkeletalMeshPreviewTab이 소유한다.
	TUniquePtr<FPreviewScene> PreviewScene;
	ASkeletalMeshActor* MeshActor = nullptr;
};