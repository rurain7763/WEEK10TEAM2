#include "EnginePCH.h"
#include "DecalActor.h"

#include "Component/BillboardComponent.h"
#include "Component/DecalComponent.h"
#include "Core/ResourcePaths.h"

ADecalActor::ADecalActor() {
	SetRootComponent(CreateDefaultSubobject<UDecalComponent>("UDecalComponent"));

	UBillboardComponent* BillboardComponent{ CreateDefaultSubobject<UBillboardComponent>("UBillboardComponent") };
	BillboardComponent->SetupAttachment(GetRootComponent());
	BillboardComponent->SetHiddenInDetails(true);
	BillboardComponent->SetTexture(ResourcePaths::DecalActorIcon);
}

UDecalComponent* ADecalActor::GetDecalComponent() const {
	return Cast<UDecalComponent>(GetRootComponent());
}

