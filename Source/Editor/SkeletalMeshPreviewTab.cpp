#include "EnginePCH.h"
#include "SkeletalMeshPreviewTab.h"
#include "Engine/World.h"
#include "Editor/Viewports/PreviewViewportPanel.h"

namespace
{
	FEditorTabSettings MakeSkeletalMeshPreviewTabSettings()
	{
		// 기존 imgui.ini 배치를 그대로 쓰도록 DockSpace 이름을 유지하고, 패널도 클래스로 묶지 않는다
		FEditorTabSettings Settings;
		Settings.Title = "Skeletal Mesh Preview";
		Settings.DockSpaceName = "SkeletalMeshPreviewDock";
		Settings.bClosable = true;
		Settings.bIsolatePanels = true;
		return Settings;
	}
}


FSkeletalMeshPreviewTab::FSkeletalMeshPreviewTab(USkeletalMesh* InSkeletalMesh) : FEditorTab(MakeSkeletalMeshPreviewTabSettings()), SkeletalMesh(InSkeletalMesh)
{
	PreviewScene = MakeUnique<FPreviewScene>();
	MeshActor = Cast<ASkeletalMeshActor>(PreviewScene->GetWorld()->SpawnActor(ASkeletalMeshActor::StaticClass()));
	MeshActor->GetSkeletalMeshComponent()->SetSkeletalMesh(SkeletalMesh);

	//TODO: 본을 선택했을때, 투명한 액터를 본위치로 이동시켜서 본을 선택한것처럼 보이게 한다.


	//TODO:TreePanel

	//ViewportPanel = AddPanel<FPreviewViewportPanel>(GetWorld(), FString("Viewport##") + SkeletalMesh->GetPath());
}
