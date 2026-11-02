#pragma once

#include "SceneComponent.h"

class FLineBatcher;
class UMaterial;

class UDecalComponent : public USceneComponent {
	DECLARE_CLASS(UDecalComponent, USceneComponent)

	REFLECT_START(UDecalComponent)
		PROPERTY(mDecalSize)
		PROPERTY(mDecalColor)
		PROPERTY(mOpacity)
		PROPERTY(mSortOrder)
		PROPERTY(mFadeScreenSize)
		PROPERTY(mDrawDebug)
	REFLECT_END()

public:
	using Super::Serialize;

public:
	UDecalComponent() = default;
	virtual ~UDecalComponent() override = default;

public:
	const FVector& GetDecalSize() const;
	void SetDecalSize(const FVector& InDecalSize);

	UMaterial* GetDecalMaterial() const;
	void SetDecalMaterial(UMaterial* InDecalMaterial);

	const FLinearColor& GetDecalColor() const;
	void SetDecalColor(const FLinearColor& InDecalColor);

	float GetOpacity() const;
	void SetOpacity(float InOpacity);

	int32 GetSortOrder() const;
	void SetSortOrder(int32 InSortOrder);

	float GetFadeScreenSize() const;
	void SetFadeScreenSize(float InFadeScreenSize);

	bool ShouldAlwaysDrawDebug() const;
	void SetDrawDebug(bool InDrawDebug);

	virtual FBox CalcLocalBounds() const override;
	void DrawDebug(FLineBatcher* LineBatcher) const;

	virtual void PostEditChangeProperty(const FProperty& Property) override;
	virtual void Serialize(FStructuredArchive::FRecord Record) override;

private:
	void ValidateParameters();

private:
	FVector mDecalSize{ 0.5f, 1.0f, 1.0f };
	UMaterial* mDecalMaterial{};
	FLinearColor mDecalColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	float mOpacity{ 1.0f };
	int32 mSortOrder{};
	float mFadeScreenSize{ 0.01f };
	bool mDrawDebug{ true };
};
