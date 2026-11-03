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
	REFLECT_END()

public:
	virtual ~UDecalComponent() = default;

	virtual void PostLoad() override;

	void SetDecalMaterial(UMaterial* InMaterial);

	UMaterial* GetDecalMaterial() const;

	void SetOpacity(float InOpacity);

	float GetOpacity() const;

	void OnTransformDirty() override;
	void PostEditChangeProperty(const FProperty& Property) override;

	FDecalSceneProxy* GetSceneProxy() const;

	FDecalSceneProxy* CreateSceneProxy(FScene* InScene);

	void DestroySceneProxy();

	void DrawDebug(FLineBatcher* LineBatcher) const;

private:
	FDecalSceneProxy* Proxy = nullptr;

	UMaterial* DecalMaterial = nullptr;
	float Opacity = 1.0f;
};
