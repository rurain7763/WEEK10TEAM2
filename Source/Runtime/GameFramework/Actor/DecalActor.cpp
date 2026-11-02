#include "EnginePCH.h"
#include "DecalActor.h"

#include "Component/DecalComponent.h"

ADecalActor::ADecalActor() {
	SetRootComponent(CreateDefaultSubobject<UDecalComponent>("UDecalComponent"));
}

UDecalComponent* ADecalActor::GetDecalComponent() const {
	return Cast<UDecalComponent>(GetRootComponent());
}

