#pragma once
#include "EnginePCH.h"
#include "DecalRenderer.h"

#include "Asset/AssetManager.h"
#include "Component/DecalComponent.h"
#include "Core/EngineTimer.h"
#include "Engine/DecalSceneProxy.h"
#include "Engine/Scene.h"
#include "Engine/World.h"
#include "Math/Frustum.h"
#include "Render/ColorWorkflow.h"
#include "Render/Material.h"
#include "Render/Mesh.h"
#include "Render/RenderCommand.h"
#include "Render/Renderer.h"
#include "Render/RenderingInfo.h"
#include "Render/RenderResourceManager.h"
#include "Render/SceneRenderer.h"
#include <algorithm>

bool FDecalRenderer::Init(FRenderer* InRenderer)
{
    Renderer = InRenderer;
    Shader = FRenderResourceManager::GetShaderProgram("Resources/Shader/ScreenSpaceDecal.hlsl");
    VolumeMesh = UAssetManager::GetAssetByPath<UStaticMesh>("Cube");

    if (!Shader || !Shader->PixelShader->GetBindings().FindConstantBuffer("DecalConstants")
        || !VolumeMesh || !VolumeMesh->GetVertexBuffer(0) || !VolumeMesh->GetIndexBuffer(0))
    {
        Shader = nullptr;
        HTR_LOG(Error, "[Decal] required shader or Cube resource not loaded");
        return false;
    }

    DecalBuffer = RenderCommand::CreateConstantBuffer(sizeof(FDecalConstants));
    MaterialBuffer = RenderCommand::CreateConstantBuffer(sizeof(FStaticMeshMaterialParams));
    if (!DecalBuffer || !MaterialBuffer || !DecalBuffer->GetBuffer() || !MaterialBuffer->GetBuffer())
    {
        Shader = nullptr;
        HTR_LOG(Error, "[Decal] constant buffer creation failed");
        return false;
    }

    PipelineState.Shader = Shader;
    PipelineState.Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    PipelineState.RasterizerState = ERasterizerState::DecalCullFront;
    PipelineState.BlendState = EBlendState::AlphaBlend;
    PipelineState.DepthStencilState = EDepthStencilState::Disabled;
    return true;
}

// 가시 데칼의 볼륨 투영 및 화면 합성
void FDecalRenderer::OnRender(const UWorld& World, const FSceneView& SceneView, const FRenderingInfo& SceneInfo)
{
    if (!Shader)
    {
        return;
    }

    // 현재 뷰에 영향을 주는 데칼 선별
    VisibleDecals.Reset();
    const FFrustumPlanes Frustum = ExtractFrustumPlanes(SceneView.ViewProjection);
    const bool bGameView = World.GetWorldType() != EWorldType::Editor;
    for (const FDecalSceneProxy* Proxy : World.GetScene().Decals)
    {
        if (!Proxy->IsVisible()
            || (bGameView && Proxy->GetComponent()->IsHiddenInGame())
            || !IsAABBInFrustum(MakeWorldBounds(Proxy->GetBounds()), Frustum))
        {
            continue;
        }
        VisibleDecals.Add(Proxy);
    }

    if (VisibleDecals.IsEmpty())
    {
        return;
    }

    // 합성 순서와 동일 순위의 결정적 정렬
    std::sort(VisibleDecals.begin(), VisibleDecals.end(),
        [](const FDecalSceneProxy* Left, const FDecalSceneProxy* Right)
        {
            if (Left->GetSortOrder() != Right->GetSortOrder())
            {
                return Left->GetSortOrder() < Right->GetSortOrder();
            }
            return Left->GetId() < Right->GetId();
        });

    // 기존 씬 컬러 유지 및 읽기 대상의 출력 연결 해제
    FRenderingInfo PassInfo = SceneInfo;
    PassInfo.ColorRenderTargets.SetNum(1);
    PassInfo.ColorRenderTargets[0].LoadOp = ERenderTargetLoadOp::Load;
    PassInfo.DepthStencil.Texture = nullptr;
    RenderCommand::BeginRenderPass(PassInfo);

    // 전용 파이프라인 및 기존 조명 자원 연결
    RenderCommand::BindPipelineState(PipelineState);
    Renderer->BindSceneLighting();
    RenderCommand::BindShaderResource(4, SceneInfo.DepthStencil.Texture, EShaderBindFlagBits::Pixel);
    RenderCommand::BindShaderResource(5, SceneInfo.ColorRenderTargets[1].Texture, EShaderBindFlagBits::Pixel);
    RenderCommand::BindConstantBuffer(1, MaterialBuffer.get(), EShaderBindFlagBits::Pixel);
    RenderCommand::BindConstantBuffer(7, DecalBuffer.get(), EShaderBindFlagBits::Vertex | EShaderBindFlagBits::Pixel);

    // 모든 데칼이 공유하는 단위 박스 연결
    RenderCommand::BindVertexBuffer(VolumeMesh->GetVertexBuffer(0));
    RenderCommand::BindIndexBuffer(VolumeMesh->GetIndexBuffer(0));
    const uint32 IndexCount = VolumeMesh->GetIndexBuffer(0)->GetIndexCount();

    // 현재 뷰의 공통 복원 정보 준비
    FDecalConstants Constants{};
    Constants.InverseViewProjection = SceneView.ViewProjection.Inverse();
    Constants.ViewportOrigin = FVector2(
        static_cast<float>(SceneInfo.ViewportSetting.StartX),
        static_cast<float>(SceneInfo.ViewportSetting.StartY));
    Constants.ViewportSize = FVector2(
        static_cast<float>(SceneInfo.ViewportSetting.Width),
        static_cast<float>(SceneInfo.ViewportSetting.Height));
    Constants.bUnlit = SceneView.RenderMode.ViewMode == EViewMode::Unlit ? 1u : 0u;

    const float TotalTime = EngineTimer::GetTotalTime();
    for (const FDecalSceneProxy* Proxy : VisibleDecals)
    {
        // 기존 머티리얼 형식을 이용한 외형 업로드
        const UMaterial* Material = Proxy->GetMaterial();
        FStaticMeshMaterialParams Params{};
        Params.BaseColor = ColorWorkflow::ToShaderColor(Material->BaseColor);
        Params.UVScrollOffset = Material->UVScrollSpeed * TotalTime;
        Params.Shininess = Material->Shininess;
        const FLinearColor SpecularColor = ColorWorkflow::ToShaderColor(Material->SpecularColor);
        Params.SpecularColor = FVector(SpecularColor.R, SpecularColor.G, SpecularColor.B);
        Params.bFlipNormalGreen = Material->bFlipNormalGreen ? 1u : 0u;
        RenderCommand::UpdateBufferData(MaterialBuffer.get(), &Params);

        // 현재 데칼의 투영 및 색상 업로드
        Constants.WorldToDecal = Proxy->GetWorldToDecal();
        Constants.DecalToWorld = Proxy->GetDecalToWorld();
        Constants.DecalColor = ColorWorkflow::ToShaderColor(Proxy->GetColor());
        RenderCommand::UpdateBufferData(DecalBuffer.get(), &Constants);

        // 미지정 Albedo의 기존 흰색 텍스처 대체
        UTexture2D* Albedo = Material->Textures.Num() > 0 && Material->Textures[0]
            ? Material->Textures[0]
            : UAssetManager::GetAssetByPath<UTexture2D>("WhiteTexture");
        RenderCommand::BindShaderResource(0, Albedo, EShaderBindFlagBits::Pixel);
        RenderCommand::BindSamplerState(0, Material->SamplerState, EShaderBindFlagBits::Pixel);

        // 미지정 Normal의 기존 평면 노멀 대체
        UTexture2D* Normal = Material->Textures.Num() > 1 && Material->Textures[1]
            ? Material->Textures[1]
            : UAssetManager::GetAssetByPath<UTexture2D>("FlatNormalTexture");
        RenderCommand::BindShaderResource(1, Normal, EShaderBindFlagBits::Pixel, true);

        // 음수 스케일의 면 방향 반전 보정
        const bool bMirrored = Constants.DecalToWorld.Determinant() < 0.0f;
        RenderCommand::SetRasterizerState(
            bMirrored ? ERasterizerState::DecalCullBack : ERasterizerState::DecalCullFront);
        RenderCommand::DrawIndexed(IndexCount);
    }

    // 다음 패스의 깊이·표면 데이터 출력 충돌 방지
    RenderCommand::UnbindShaderResource(4, EShaderBindFlagBits::Pixel);
    RenderCommand::UnbindShaderResource(5, EShaderBindFlagBits::Pixel);
    RenderCommand::EndRenderPass();

    // 다음 렌더링을 위한 기본 상태 복원
    RenderCommand::SetRasterizerState(ERasterizerState::SolidBack);
    RenderCommand::SetBlendState(EBlendState::Opaque);
    RenderCommand::SetDepthStencilState(EDepthStencilState::Default);
}