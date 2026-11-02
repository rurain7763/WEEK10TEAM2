#include "EnginePCH.h"
#include "SkeletalMeshPreviewTab.h"

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

SkeletalMeshPreviewTab::SkeletalMeshPreviewTab() : FEditorTab(MakeSkeletalMeshPreviewTabSettings())
{


}
