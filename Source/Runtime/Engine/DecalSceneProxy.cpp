#include "EnginePCH.h"

#include "DecalSceneProxy.h"
#include "Component/DecalComponent.h"

FDecalSceneProxy::FDecalSceneProxy(FScene* InScene, UDecalComponent* InComponent)
	: Scene(InScene)
	, Component(InComponent)
{
}

void FDecalSceneProxy::UpdateTransform()
{
	InvWorld = Component->GetWorldMatrix().Inverse();
}

void FDecalSceneProxy::UpdateMaterial()
{
	Material = Component->GetDecalMaterial();
}

void FDecalSceneProxy::UpdateOpacity()
{
	Opacity = Component->GetOpacity();
}

FScene* FDecalSceneProxy::GetScene() const
{
	return Scene;
}

UDecalComponent* FDecalSceneProxy::GetComponent() const
{
	return Component;
}

UMaterial* FDecalSceneProxy::GetDecalMaterial() const
{
	return Material;
}

const FMatrix& FDecalSceneProxy::GetInvWorldMatrix() const
{
	return InvWorld;
}

float FDecalSceneProxy::GetOpacity() const
{
	return Opacity;
}
