#pragma once
#pragma once

#include "Core/Types.h"
#include "Containers/Array.h"
#include "Math/EngineMath.h"
#include "Math/Color.h"
#include "Render/Buffer.h"
#include "Render/PipelineState.h"

class FRenderer;
class FDecalSceneProxy;
class UStaticMesh;
class UWorld;
struct FSceneView;
struct FRenderingInfo;

// 화면 위치 복원 및 데칼 볼륨 투영의 GPU 상수
struct FDecalConstants
{
    FMatrix InverseViewProjection;
    FMatrix WorldToDecal;
    FMatrix DecalToWorld;
    FLinearColor DecalColor;
    FVector2 ViewportOrigin;
    FVector2 ViewportSize;
    uint32 bUnlit = 0;
    float Padding[3]{};
};
static_assert(sizeof(FDecalConstants) == 240);

// 데칼 볼륨의 GPU 자원 및 합성 드로우 관리
class FDecalRenderer
{
public:
    bool Init(FRenderer* InRenderer);
    void OnRender(const UWorld& World, const FSceneView& SceneView, const FRenderingInfo& SceneInfo);

private:
    // 기존 렌더러·셰이더·공유 메시의 비소유 참조
    FRenderer* Renderer = nullptr;
    FShaderProgram* Shader = nullptr;
    UStaticMesh* VolumeMesh = nullptr;

    // 데칼 전용 상태 및 상수 버퍼 소유
    FPipelineState PipelineState;
    TUniquePtr<FConstantBuffer> DecalBuffer;
    TUniquePtr<FConstantBuffer> MaterialBuffer;

    // 뷰별 가시 목록의 메모리 재사용
    TArray<const FDecalSceneProxy*> VisibleDecals;
};