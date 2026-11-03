#include "EnginePCH.h"
#include "DecalComponent.h"
#include "Engine/DecalSceneProxy.h"

void UDecalComponent::PostLoad()
{
	Proxy->UpdateMaterial();
	Proxy->UpdateTransform();
	Proxy->UpdateOpacity();
}

void UDecalComponent::SetDecalMaterial(UMaterial* InMaterial)
{
	DecalMaterial = InMaterial;

	if (Proxy)
	{
		Proxy->UpdateMaterial();
	}
}

UMaterial* UDecalComponent::GetDecalMaterial() const
{
	return DecalMaterial;
}

void UDecalComponent::SetOpacity(float InOpacity)
{
	Opacity = InOpacity;

	if (Proxy)
	{
		Proxy->UpdateOpacity();
	}
}

float UDecalComponent::GetOpacity() const
{
	return Opacity;
}

void UDecalComponent::OnTransformDirty()
{
	if (!Proxy)
	{
		return;
	}

	Proxy->UpdateTransform();
}

void UDecalComponent::PostEditChangeProperty(const FProperty& Property)
{
	Super::PostEditChangeProperty(Property);

	if (Property.Name == "DecalMaterial")
	{
		SetDecalMaterial(DecalMaterial);
	}
	else if (Property.Name == "Opacity")
	{
		SetOpacity(Opacity);
	}
}

FDecalSceneProxy* UDecalComponent::CreateSceneProxy(FScene* InScene)
{
	if (Proxy && Proxy->GetScene() == InScene)
	{
		return Proxy;
	}

	Proxy = new FDecalSceneProxy(InScene, this);
	Proxy->UpdateMaterial();
	Proxy->UpdateTransform();
	Proxy->UpdateOpacity();

	return Proxy;
}

void UDecalComponent::DestroySceneProxy()
{
	if (Proxy)
	{
		delete Proxy;
		Proxy = nullptr;
	}
}

void UDecalComponent::DrawDebug(FLineBatcher* LineBatcher) const
{
	FBox LocalBox;
	LocalBox.Min = FVector(-0.5f, -0.5f, -0.5f);
	LocalBox.Max = FVector(0.5f, 0.5f, 0.5f);

	LineBatcher->AddOrientedBox(LocalBox, GetWorldMatrix(), FVector4(1.f, 0.f, 0.f, 1.f));
}

FDecalSceneProxy* UDecalComponent::GetSceneProxy() const
{
	return Proxy;
}
