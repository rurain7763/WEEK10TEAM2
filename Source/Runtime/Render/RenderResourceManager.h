#pragma once

#include "Shader.h"
#include "ViewMode.h"

class FRenderResourceManager
{
public:
	static FRenderResourceManager& Get()
	{
		static FRenderResourceManager* Instance = new FRenderResourceManager();
		return *Instance;
	}

	static void Init()
	{
		Get().ScanShaders("Resources/Shader");
		Get().LoadUberLitPermutations();
	}

	void ScanShaders(const fs::path& ShaderRoot);

	static void Shutdown();

	static FShaderProgram* GetShaderProgram(const FString& InPath);
	static FShaderProgram* GetUberLitProgram(const FViewRenderMode& Mode, bool bHasNormalMap, bool bReceiveShadow = true, bool bUseSkinning = false);

private:
	static FString GetUberLitSuffix(const FViewRenderMode& Mode, bool bHasNormalMap = false, bool bReceiveShadow = true, bool bUseSkinning = false);

	void LoadOrCompileShader(const FString& Path, const FString& PermutationSuffix = "", const D3D_SHADER_MACRO* Defines = nullptr, const TArray<D3D11_INPUT_ELEMENT_DESC>& InputLayoutDescs = {});

	void LoadUberLitPermutations();

	TMap<FString, TUniquePtr<FVertexShader>> VertexShaderMap;
	TMap<FString, TUniquePtr<FPixelShader>>  PixelShaderMap;
	TMap<FString, TUniquePtr<FShaderProgram>> ShaderProgramMap;
};