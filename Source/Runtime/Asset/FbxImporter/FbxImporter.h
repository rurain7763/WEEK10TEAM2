#pragma once

#include "Asset/ModelData.h"
#include "Containers/Map.h"
#include <filesystem>
#include <fbxsdk.h>

class FFbxTextureResolver;

class FFbxImporter
{
public:
static bool LoadFromFile(const FString& Path, FModelData& OutModelData,
    const TArray<std::filesystem::path>& AdditionalTextureDirectories = {});
private:
	struct FMeshNode
	{
		fbxsdk::FbxNode* FbxNode;
		int32 NodeIndex;
	};

	struct FSoftModelVertexInfluences
	{
		TArray<int32> BoneIndices;
		TArray<float> Weights;
	};

	struct FTriangleFace
	{
		uint32 VertexIndices[3];
		uint32 MaterialSlotIndex;
	};

	static void BuildNodeHierarchy(int32 ParentNodeIndex, fbxsdk::FbxNode* Current, TMap<uint64, int32>& NodeToIndexMap, 
		TMap<int32, int32>& NodeIndexToBoneIndexMap, TArray<FModelNode>& OutNodes, TArray<FModelBone>& OutBones, 
		TArray<FMeshNode>& OutMeshNodes);

	static void ProcessMeshNode(const FFbxTextureResolver& TextureResolver,
		const TMap<uint64, int32>& NodeToIndexMap,
		const TMap<int32, int32>& NodeIndexToBoneIndexMap, TArray<FModelBone>& Bones,
		const FMeshNode& MeshNode, FModelMeshData& OutModelMeshData,
		TArray<FModelVertexInfluences>& OutVertexInfluencesList, TArray<FModelMesh>& OutMeshs);


	static void ParseMaterial(const FFbxTextureResolver& TextureResolver,
		fbxsdk::FbxSurfaceMaterial* Material, FMeshMaterialSlot& OutMaterial);

	static void ParseMesh(const FFbxTextureResolver& TextureResolver, fbxsdk::FbxMesh* Mesh,
		TArray<FVertexPNCT>& OutVertices, TArray<int32>& OutVertexToControlPoints,
		TArray<FMeshMaterialSlot>& OutMaterials, TArray<FTriangleFace>& OutFaces);

	static void CompactSoftSkin(FSoftModelVertexInfluences& Skin);

	static void ParseSkin(const TArray<int32>& VertexToControlPoints, const TMap<uint64, int32>& NodeToIndexMap, 
		const TMap<int32, int32>& NodeIndexToBoneIndexMap, const FMeshNode& MeshNode, TArray<FModelBone>& Bones, 
		TArray<FModelVertexInfluences>& OutVertexInfluencesList, FMatrix& OutBindGlobalTransform);
};
