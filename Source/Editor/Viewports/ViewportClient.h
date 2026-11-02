#pragma once

#include "Editor/LevelEditor/MultipleViewports/Core/MultipleViewports.h"

// 한 프레임 동안 활성 View 하나에 전달하는 입력값의 묶음. 이벤트 처리나 중계 책임은 갖지 않는다.
struct FViewportInput
{
    float DeltaTime = 0.0f;
    float MouseSensitivity = 0.0f;
    // 우클릭 + 휠로 바뀌는 카메라 이동 속도. Client가 바꾼 값을 호출자가 다시 읽는다.
    float MoveSpeed = 0.0f;
    int32 WheelDelta = 0;
    // 이 View가 우클릭 Capture 중이고 우클릭이 눌려 있다.
    bool bCameraCaptured = false;
    // 마우스가 이 View 위에 있다.
    bool bPointerOver = false;
};

// View 하나가 그릴 카메라를 제공한다. 배치(Rect)와 렌더 대상은 FViewport와 Adapter가 맡는다.
class FViewportClient
{
public:
    virtual ~FViewportClient() = default;
    virtual const FViewCamera& GetCamera() const = 0;
    // 이 View에 전달된 입력으로 카메라를 갱신한다. 기본은 입력을 받지 않는다.
    virtual void UpdateInput(FViewportInput& Input, const FRect& Rect) {}
    // View Rect가 유효한 크기로 바뀌었을 때 투영값을 보정한다.
    virtual void OnRectChanged(const FRect& Rect) {}
};
