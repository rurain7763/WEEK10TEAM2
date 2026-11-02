#include "EnginePCH.h"
#include "LevelEditorTab.h"


namespace
{
	FEditorTabSettings MakeLevelTabSettings()
	{
		// 기존 imgui.ini 배치를 그대로 쓰도록 DockSpace 이름을 유지하고, 패널도 클래스로 묶지 않는다
		FEditorTabSettings Settings;
		Settings.Title = "Level";
		Settings.DockSpaceName = "EngineDockingSpace";
		Settings.bClosable = false;
		Settings.bIsolatePanels = false;
		return Settings;
	}
}

FLevelEditorTab::FLevelEditorTab()
	: FEditorTab(MakeLevelTabSettings())
{



}
