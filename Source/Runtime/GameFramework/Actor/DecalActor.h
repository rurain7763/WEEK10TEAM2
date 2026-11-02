#pragma once

#include "GameFramework/Actor.h"

class UDecalComponent;

class ADecalActor final : public AActor {
	DECLARE_CLASS(ADecalActor, AActor)

public:
	ADecalActor();
	virtual ~ADecalActor() override = default;

public:
	UDecalComponent* GetDecalComponent() const;
};
