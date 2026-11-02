#pragma once
#include "Editor/FEditorTab.h"

#include <functional>
class FViewportsPanel;


class FLevelEditorTab : public FEditorTab
{
public:
	FLevelEditorTab();
	virtual ~FLevelEditorTab() = default;;

	FLevelEditorTab(const FLevelEditorTab&) = delete;
	FLevelEditorTab& operator=(const FLevelEditorTab&) = delete;

	void SetViewportsPanel(FViewportsPanel* InViewportsPanel) { ViewportsPanel = InViewportsPanel; }
	void SetGizmo(const FGizmo* InGizmo) { Gizmo = InGizmo; }
	

private:
	//뷰포트를 담는 패널. FLevelEditorTab이 소유한다.
	FViewportsPanel* ViewportsPanel = nullptr;
	const FGizmo* Gizmo = nullptr;


};