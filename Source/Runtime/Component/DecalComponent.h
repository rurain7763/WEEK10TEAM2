#pragma once

#include "SceneComponent.h"
#include "Render/Material.h"
#include "Render/LineBatcher.h"

class FScene;
class FDecalSceneProxy;

class UDecalComponent : public USceneComponent
{
	DECLARE_CLASS(UDecalComponent, USceneComponent)

	REFLECT_START(ClassName)
		PROPERTY(DecalMaterial)
		PROPERTY(Opacity)
		PROPERTY(FadeInStartDelay)
		PROPERTY(FadeInDuration)
		PROPERTY(FadeStartDelay)
		PROPERTY(FadeDuration)
		PROPERTY(bDestroyOwnerAfterFade)
	REFLECT_END()

public:
	UDecalComponent();
	virtual ~UDecalComponent() = default;

	virtual void PostLoad() override;

	virtual void TickComponent(float DeltaTime) override;

	void SetDecalMaterial(UMaterial* InMaterial);

	UMaterial* GetDecalMaterial() const;

	void SetOpacity(float InOpacity);

	float GetOpacity() const;
	float GetFadeAlpha() const;

	void OnTransformDirty() override;
	void PostEditChangeProperty(const FProperty& Property) override;

	FDecalSceneProxy* GetSceneProxy() const;

	FDecalSceneProxy* CreateSceneProxy(FScene* InScene);

	void DestroySceneProxy();

	void DrawDebug(FLineBatcher* LineBatcher) const;
	
	inline bool IsPersistent() const
	{
		return FadeStartDelay <= 0.f && FadeDuration <= 0.f;
	}

private:
	FDecalSceneProxy* Proxy = nullptr;

	UMaterial* DecalMaterial = nullptr;
	float Opacity = 1.0f;

	float FadeInStartDelay = 0.f;
	float FadeInDuration = 0.f;
	float FadeStartDelay = 0.f;
	float FadeDuration = 0.f;
	bool bDestroyOwnerAfterFade = true;

	float ElapsedTime = 0.f;
	float FadeAlpha = 1.f;
};
