#include "EnginePCH.h"
#include "DecalComponent.h"

#include "Render/LineBatcher.h"
#include "Render/Material.h"

#include <cfloat>

const FVector& UDecalComponent::GetDecalSize() const {
	return mDecalSize;
}

void UDecalComponent::SetDecalSize(const FVector& InDecalSize) {
	mDecalSize = InDecalSize;
	ValidateParameters();
}

UMaterial* UDecalComponent::GetDecalMaterial() const {
	return mDecalMaterial;
}

void UDecalComponent::SetDecalMaterial(UMaterial* InDecalMaterial) {
	mDecalMaterial = InDecalMaterial;
}

const FLinearColor& UDecalComponent::GetDecalColor() const {
	return mDecalColor;
}

void UDecalComponent::SetDecalColor(const FLinearColor& InDecalColor) {
	mDecalColor = InDecalColor;
}

float UDecalComponent::GetOpacity() const {
	return mOpacity;
}

void UDecalComponent::SetOpacity(float InOpacity) {
	mOpacity = InOpacity;
	ValidateParameters();
}

int32 UDecalComponent::GetSortOrder() const {
	return mSortOrder;
}

void UDecalComponent::SetSortOrder(int32 InSortOrder) {
	mSortOrder = InSortOrder;
}

float UDecalComponent::GetFadeScreenSize() const {
	return mFadeScreenSize;
}

void UDecalComponent::SetFadeScreenSize(float InFadeScreenSize) {
	mFadeScreenSize = InFadeScreenSize;
	ValidateParameters();
}

bool UDecalComponent::ShouldAlwaysDrawDebug() const {
	return mDrawDebug;
}

void UDecalComponent::SetDrawDebug(bool InDrawDebug) {
	mDrawDebug = InDrawDebug;
}

FBox UDecalComponent::CalcLocalBounds() const {
	return FBox{ -mDecalSize, mDecalSize };
}

void UDecalComponent::DrawDebug(FLineBatcher* LineBatcher) const {
	if (!LineBatcher) {
		return;
	}

	const FMatrix WorldMatrix{ GetWorldMatrix() };
	const FVector4 Color{ 0.0f, 0.8f, 1.0f, 1.0f };
	LineBatcher->AddOrientedBox(CalcLocalBounds(), WorldMatrix, Color);

	constexpr float ArrowHalfLength{ 0.5f };
	constexpr float ArrowHeadLength{ 0.25f };
	constexpr float ArrowHeadWidth{ 0.2f };
	const FVector Center{ GetWorldLocation() };
	
	const FQuat Rotation{ GetWorldRotation().Quaternion() };
	const FVector Forward{ Rotation.GetForwardVector() };
	const FVector Right{ Rotation.GetRightVector() };

	const FVector Start{ Center - Forward * ArrowHalfLength };
	const FVector End{ Center + Forward * ArrowHalfLength };
	const FVector HeadLeft{ End - Forward * ArrowHeadLength + Right * ArrowHeadWidth };
	const FVector HeadRight{ End - Forward * ArrowHeadLength - Right * ArrowHeadWidth };

	LineBatcher->AddLine(Start, End, Color);
	LineBatcher->AddLine(End, HeadLeft, Color);
	LineBatcher->AddLine(End, HeadRight, Color);
}

void UDecalComponent::PostEditChangeProperty(const FProperty& Property) {
	Super::PostEditChangeProperty(Property);
	ValidateParameters();
}

void UDecalComponent::Serialize(FStructuredArchive::FRecord Record) {
	Super::Serialize(Record);
	UMaterial::SerializeMaterialReference(Record.EnterField("DecalMaterial"), mDecalMaterial);

	if (Record.GetUnderlyingArchive().IsLoading()) {
		ValidateParameters();
	}
}

void UDecalComponent::ValidateParameters() {
	mDecalSize.X = FMath::Clamp(mDecalSize.X, 0.001f, FLT_MAX);
	mDecalSize.Y = FMath::Clamp(mDecalSize.Y, 0.001f, FLT_MAX);
	mDecalSize.Z = FMath::Clamp(mDecalSize.Z, 0.001f, FLT_MAX);
	mOpacity = FMath::Clamp(mOpacity, 0.0f, 1.0f);
	mFadeScreenSize = FMath::Clamp(mFadeScreenSize, 0.0f, 1.0f);
}
