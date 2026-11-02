#pragma once

#include "Editor/Viewports/EditorViewportClient.h"
#include "Editor/Viewports/GameViewportClient.h"

#include "Math/Frustum.h"
#include "Math/Matrix.h"
#include "Render/ViewMode.h"

// 화면의 View 하나. 배치된 Rect와 표시 모드를 갖고, 어느 Client의 카메라로 그릴지 고른다.
// 편집 Client는 계속 보관하고, PIE 중에는 Game Client가 대신 그린다(언리얼의 PIE 뷰포트 전환과 같다).
class FViewport
{
public:
    void SetEditorClient(TUniquePtr<FEditorViewportClient> Client) { EditorClient = std::move(Client); }
    FEditorViewportClient* GetEditorClient() const { return EditorClient.get(); }

    // Game Client가 없으면 만들고 반환한다. 이후 이 View는 Game Client의 카메라로 그린다.
    FGameViewportClient& BeginPlay();
    // Game Client를 지우고 편집 Client로 되돌린다.
    void EndPlay() { GameClient.reset(); }
    FGameViewportClient* GetGameClient() const { return GameClient.get(); }
    bool HasGameClient() const { return GameClient != nullptr; }
    // 지금 이 View를 그리는 Client. Game Client가 있으면 그쪽이다.
    FViewportClient* GetActiveClient() const;

    // Adapter가 레이아웃을 계산할 때마다 갱신한다.
    void SetRect(const FRect& InRect) { Rect = InRect; }
    const FRect& GetRect() const { return Rect; }

    void SetViewRenderMode(const FViewRenderMode& Value) { ViewRenderMode = Value; }
    FViewRenderMode GetViewRenderMode() const { return ViewRenderMode; }

    // 활성 Client의 카메라. Client가 없으면 기본값이다.
    const FViewCamera& GetCamera() const;
    // 직교 View의 논리 위치는 유지하고 렌더·컬링·피킹용 깊이 범위만 확장한 카메라.
    FViewCamera GetRenderCamera() const;

    // 현재 호출에서 계산한 행렬·절두체.
    struct FPreparedView
    {
        FFrustumPlanes Frustum{};
        FMatrix ViewProjection{};
    };
    // 호출할 때마다 현재 카메라로 VP와 절두체를 계산한다.
    const FPreparedView& PrepareView() const;

    FVector GetCameraLocation() const;
    FVector GetCameraForward() const;
    FVector GetCameraUp() const;
    // 카메라를 향하는 Billboard 월드 행렬을 만든다.
    FMatrix BuildBillboardMatrix(const FVector& WorldPosition, float Width, float Height) const;
    bool IsOrthographic() const { return GetCamera().Projection.Mode == EProjectionMode::Orthographic; }
    // 편집 Client의 프리셋과 실제로 그리는 카메라 방향으로 Grid 평면을 고른다. 편집 Client가 있어야 한다.
    EGridPlane GetGridPlane() const { return EditorClient->GetGridPlane(GetCamera().Transform.Rotation); }
    const FCameraProjection& GetProjection() const { return GetCamera().Projection; }
    // View Rect 기준 로컬 좌표를 렌더 카메라로 역투영한다.
    FRay Deproject(FVector2 ViewLocalPosition) const;

private:
    TUniquePtr<FEditorViewportClient> EditorClient;
    TUniquePtr<FGameViewportClient> GameClient;
    FRect Rect{};
    FViewRenderMode ViewRenderMode{};
    mutable FPreparedView Prepared{};
};
