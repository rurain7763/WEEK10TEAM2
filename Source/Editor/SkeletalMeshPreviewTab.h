#pragma once
#include "Editor/FEditorTab.h"

#include <functional>

class FViewportsPanel;

class SkeletalMeshPreviewTab : public FEditorTab
{
public:
	SkeletalMeshPreviewTab();
	virtual ~SkeletalMeshPreviewTab() = default;;
	SkeletalMeshPreviewTab(const SkeletalMeshPreviewTab&) = delete;
	SkeletalMeshPreviewTab& operator=(const SkeletalMeshPreviewTab&) = delete;
private:
	FViewportsPanel* ViewportPanel = nullptr;
	const FGizmo* Gizmo = nullptr;


};