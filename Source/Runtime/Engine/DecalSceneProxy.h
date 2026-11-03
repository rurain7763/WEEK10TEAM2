#pragma once

#include "Math/Matrix.h"

class FScene;
class UDecalComponent;
class UMaterial;

class FDecalSceneProxy
{
public:
	FDecalSceneProxy(FScene* InScene, UDecalComponent* InComponent);
	
	void UpdateTransform();
	void UpdateMaterial();
	void UpdateOpacity();
	void UpdateFadeAlpha();

	FScene* GetScene() const;
	UDecalComponent* GetComponent() const;
	UMaterial* GetDecalMaterial() const;
	const FMatrix& GetInvWorldMatrix() const;
	float GetOpacity() const;
	float GetFadeAlpha() const;

private:
	FScene* Scene = nullptr;
	UDecalComponent* Component = nullptr;

	UMaterial* Material = nullptr;
	FMatrix InvWorld;
	float Opacity;
	float FadeAlpha;
};
