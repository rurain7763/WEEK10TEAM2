#pragma once
#include "EnginePCH.h"
#include "Editor/EditorUI/EditorPanel.h"

class FViewport;
struct FViewportInput;
class FEditorViewportClient;
class FGizmo;
class UWorld;
class UObject;

struct FEditorTabSettings
{
	FString Title;
	// imgui.ini에 저장되는 도킹 배치의 키. 비우면 Title을 쓴다
	FString DockSpaceName;

	//닫을 수 있는 탭인지(레벨 탭의 경우 닫을 수 없음)
	bool bClosable = true;

	// 다른 탭의 패널과 서로 도킹되지 않게 막는다
	bool bIsolatePanels = true;
};


class FEditorTab
{
public:
	explicit FEditorTab(FEditorTabSettings InSettings);
	virtual ~FEditorTab() = default;
	FEditorTab(const FEditorTab&) = delete;
	FEditorTab& operator=(const FEditorTab&) = delete;

	// 필요한 Panel을 추가한 뒤 raw pointer를 반환한다(소유권 = Tab)
	template <typename T, typename... TArgs>
	T* AddPanel(TArgs&&... Args)
	{
		TUniquePtr<T> NewPanel = MakeUnique<T>(std::forward<TArgs>(Args)...);
		T* Ret = NewPanel.get();
		Ret->Init();
		Panels.Add(std::move(NewPanel));

		return Ret;
	}

	void Tick(float DeltaTime);
	void RenderPanels();

	const char* GetTitle() const { return Settings.Title.c_str(); }
	bool CanClose() const { return Settings.bClosable; }
	const TArray<TUniquePtr<IEditorPanel>>& GetPanels() const { return Panels; }
	//탭의 DockSpace 이름으로 ImGui ID를 계산해서 DockSpaceId에 넣어 두는 함수
	void ResolveDockSpaceId();
	//선택된 탭은 DockSpace를 그리고, 나머지는 도킹 관계만 유지한다
	void SubmitDockSpace(bool bVisible, ImGuiDockNodeFlags Flags);


	// 탭 바의 닫기 버튼이 내린다
	bool* GetOpenFlag() { return CanClose() ? &bOpen : nullptr; }

	//열라는 요청이 들어왔는지 확인하고, 요청이 있으면 false로 바꾸고 true를 반환한다.
	bool ConsumeFocusRequest() { const bool bRequested = bFocusRequested; bFocusRequested = false; return bRequested; }
protected:
	// imgui.ini에 이 탭의 배치가 없을 때 한 번 부른다. 기본은 모든 패널을 한 노드에 겹친다
	// DockBuilder로 DockSpaceRoot를 나누고 패널 창 이름으로 배치한다
	void BuildDefaultLayout(ImGuiID DockSpaceRoot);

private:
	//이 탭이 가지고있는 패널들
	TArray<TUniquePtr<IEditorPanel>> Panels;
	//탭 설정
	FEditorTabSettings Settings;

	ImGuiWindowClass WindowClass;
	ImGuiID DockSpaceId = 0;
	bool bOpen = true;
	bool bFocusRequested = false;

};