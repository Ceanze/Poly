#include "SceneAssetImporter.h"

#include "Poly/Model/Vertex.h"
#include "Poly/Poly/Format.h"
#include "Poly/Resources/AssetHandler.h"
#include "Poly/Resources/AssetTypes/MaterialAsset.h"
#include "Poly/Resources/AssetTypes/MeshAsset.h"
#include "Poly/Resources/GeometryPool.h"
#include "Poly/Resources/PathUtils.h"
#include "Poly/Resources/VFS/VirtualFileSystem.h"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/mesh.h>
#include <assimp/pbrmaterial.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

namespace
{
	glm::mat4 ConvertAiMatToGLM(const aiMatrix4x4& mat)
	{
		return glm::mat4(mat.a1, mat.b1, mat.c1, mat.d1,
		                 mat.a2, mat.b2, mat.c2, mat.d2,
		                 mat.a3, mat.b3, mat.c3, mat.d3,
		                 mat.a4, mat.b4, mat.c4, mat.d4);
	}

	Poly::Material::Type ConvertTextureType(aiTextureType aiType)
	{
		using namespace Poly;

		switch (aiType)
		{
		case aiTextureType_BASE_COLOR:
			return Material::Type::ALBEDO;
		case aiTextureType_DIFFUSE:
			return Material::Type::ALBEDO;

		case aiTextureType_NORMAL_CAMERA:
			return Material::Type::NORMAL;
		case aiTextureType_NORMALS:
			return Material::Type::NORMAL;
		case aiTextureType_HEIGHT:
			return Material::Type::NORMAL;

		case aiTextureType_AMBIENT_OCCLUSION:
			return Material::Type::AMBIENT_OCCLUSION;
		case aiTextureType_AMBIENT:
			return Material::Type::AMBIENT_OCCLUSION;

		case aiTextureType_METALNESS:
			return Material::Type::METALIC;
		case aiTextureType_REFLECTION:
			return Material::Type::METALIC;

		case aiTextureType_DIFFUSE_ROUGHNESS:
			return Material::Type::ROUGHNESS;
		case aiTextureType_SHININESS:
			return Material::Type::ROUGHNESS;

		case aiTextureType_UNKNOWN:
			return Material::Type::COMBINED;

		default:
			return Material::Type::NONE;
		}
	}

	[[nodiscard]] Poly::AssetHandle<Poly::TextureAsset> LoadAssimpMaterial(
	    const aiMaterial* pMaterial, aiTextureType type, uint32 index, std::string_view vfsPath)
	{
		using namespace Poly;

		aiString path;
		if (pMaterial->GetTexture(type, index, &path) != AI_SUCCESS)
		{
			POLY_CORE_WARN("Failed to get texture {} with index {}", path.C_Str(), index);
			return Poly::AssetHandle<Poly::TextureAsset>();
		}

		std::string texturePath = PathUtils::GetDirectoryPath(vfsPath) + "/" + path.C_Str();

		return AssetHandler::Load<TextureAsset>(texturePath);
	}
} // namespace

namespace Poly
{
	std::vector<std::string> SceneAssetImporter::GetSupportedExtensions() const
	{
		return {".gltf", ".glb", ".fbx", ".obj"};
	}

	bool SceneAssetImporter::Import(std::string_view vfsPath, AssetRegistry& registry)
	{
		std::string resolvedPath = VirtualFileSystem::Resolve(vfsPath);
		if (resolvedPath.empty())
		{
			POLY_CORE_ERROR("Could not resolve path {}", vfsPath);
			return false;
		}

		Assimp::Importer importer;
		const aiScene*   pScene = importer.ReadFile(resolvedPath, aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		if (!pScene)
		{
			POLY_CORE_WARN("Could not open file at path {}", resolvedPath);
			return false;
		}

		SceneAsset sceneAsset(AssetID{vfsPath});
		for (unsigned i = 0u; i < pScene->mNumMeshes; i++)
		{
			aiMesh* pMesh = pScene->mMeshes[i];
			std::string path = Poly::Format("{}#{}", vfsPath, pMesh->mName.length > 0 ? pMesh->mName.C_Str() : std::to_string(i));
			AssetID     assetID(path);
			if (registry.IsLoaded<MeshAsset>(assetID))
				POLY_CORE_WARN("Mesh '{}' in {} shares its sub-asset path with another mesh, references to it by path will be ambiguous", pMesh->mName.C_Str(), vfsPath);
			registry.RegisterPath(assetID, path);
			sceneAsset.AddMeshAsset(registry.Emplace<MeshAsset>(assetID, LoadMesh(assetID, pMesh)));
		}

		for (unsigned i = 0u; i < pScene->mNumMaterials; i++)
		{
			aiMaterial* pMaterial = pScene->mMaterials[i];
			std::string path = Poly::Format("{}#{}", vfsPath, pMaterial->GetName().length > 0 ? pMaterial->GetName().C_Str() : std::to_string(i));
			AssetID     assetID(path);
			if (registry.IsLoaded<MaterialAsset>(assetID))
				POLY_CORE_WARN("Material '{}' in {} shares its sub-asset path with another material, references to it by path will be ambiguous", pMaterial->GetName().C_Str(), vfsPath);
			registry.RegisterPath(assetID, path);
			sceneAsset.AddMaterialAsset(registry.Emplace<MaterialAsset>(assetID, LoadMaterial(assetID, pMaterial, vfsPath)));
		}

		sceneAsset.SetRootNode(sceneAsset.AddNode(ImportNode(sceneAsset, pScene, pScene->mRootNode)));

		registry.Emplace(sceneAsset.GetID(), std::move(sceneAsset));

		return true;
	}

	MaterialAsset SceneAssetImporter::LoadMaterial(const AssetID& assetID, const aiMaterial* pMaterial, std::string_view vfsPath)
	{
		MaterialAsset  materialAsset(assetID);
		MaterialValues materialValues = {};

		// Constants
		// Metallic
		ai_real metallic = 0.0f;
		if (pMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == aiReturn_SUCCESS)
		{
			materialValues.Metallic = metallic;
		}

		// Roughness
		ai_real roughness = 0.0f;
		if (pMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == aiReturn_SUCCESS)
		{
			materialValues.Roughness = roughness;
		}

		// Albedo
		aiColor4D diffuse;
		if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == aiReturn_SUCCESS)
		{
			materialValues.Albedo.r = diffuse.r;
			materialValues.Albedo.g = diffuse.g;
			materialValues.Albedo.b = diffuse.b;
			materialValues.Albedo.a = diffuse.a;
		}

		// TODO: Support all Assimp texture types
		// Textures
		// Albedo
		if (pMaterial->GetTextureCount(aiTextureType_BASE_COLOR) > 0)
			materialAsset.SetTexture(Material::Type::ALBEDO, LoadAssimpMaterial(pMaterial, aiTextureType_BASE_COLOR, 0, vfsPath));
		else if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 1)
			materialAsset.SetTexture(Material::Type::ALBEDO, LoadAssimpMaterial(pMaterial, aiTextureType_DIFFUSE, 1, vfsPath));
		else if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
			materialAsset.SetTexture(Material::Type::ALBEDO, LoadAssimpMaterial(pMaterial, aiTextureType_DIFFUSE, 0, vfsPath));

		// Normal
		if (pMaterial->GetTextureCount(aiTextureType_NORMAL_CAMERA) > 0)
			materialAsset.SetTexture(Material::Type::NORMAL, LoadAssimpMaterial(pMaterial, aiTextureType_NORMAL_CAMERA, 0, vfsPath));
		else if (pMaterial->GetTextureCount(aiTextureType_NORMALS) > 0)
			materialAsset.SetTexture(Material::Type::NORMAL, LoadAssimpMaterial(pMaterial, aiTextureType_NORMALS, 0, vfsPath));
		else if (pMaterial->GetTextureCount(aiTextureType_HEIGHT) > 0)
			materialAsset.SetTexture(Material::Type::NORMAL, LoadAssimpMaterial(pMaterial, aiTextureType_HEIGHT, 0, vfsPath));

		// Ambient Occlusion
		if (pMaterial->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION) > 0)
			materialAsset.SetTexture(Material::Type::AMBIENT_OCCLUSION, LoadAssimpMaterial(pMaterial, aiTextureType_AMBIENT_OCCLUSION, 0, vfsPath));
		else if (pMaterial->GetTextureCount(aiTextureType_AMBIENT) > 0)
			materialAsset.SetTexture(Material::Type::AMBIENT_OCCLUSION, LoadAssimpMaterial(pMaterial, aiTextureType_AMBIENT, 0, vfsPath));

		// Metallic
		if (pMaterial->GetTextureCount(aiTextureType_METALNESS) > 0)
			materialAsset.SetTexture(Material::Type::METALIC, LoadAssimpMaterial(pMaterial, aiTextureType_METALNESS, 0, vfsPath));
		else if (pMaterial->GetTextureCount(aiTextureType_REFLECTION) > 0)
			materialAsset.SetTexture(Material::Type::METALIC, LoadAssimpMaterial(pMaterial, aiTextureType_REFLECTION, 0, vfsPath));

		// Roughness
		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0)
			materialAsset.SetTexture(Material::Type::ROUGHNESS, LoadAssimpMaterial(pMaterial, aiTextureType_DIFFUSE_ROUGHNESS, 0, vfsPath));
		else if (pMaterial->GetTextureCount(aiTextureType_SHININESS) > 0)
			materialAsset.SetTexture(Material::Type::ROUGHNESS, LoadAssimpMaterial(pMaterial, aiTextureType_SHININESS, 0, vfsPath));

		// Combined occlusion metallic roughness
		if (pMaterial->GetTextureCount(aiTextureType_UNKNOWN) > 0)
		{
			materialAsset.SetTexture(Material::Type::COMBINED, LoadAssimpMaterial(pMaterial, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, vfsPath));
			materialValues.IsCombined = 1.0;
		}

		materialAsset.SetMaterialValues(std::move(materialValues));
		return materialAsset;
	}

	MeshAsset SceneAssetImporter::LoadMesh(const AssetID& assetID, const aiMesh* pMesh)
	{
		std::vector<Vertex> vertices(pMesh->mNumVertices);
		std::vector<uint32> indices(pMesh->mNumFaces * 3);

		for (uint32 i = 0; i < pMesh->mNumVertices; i++)
		{
			vertices[i].Position.x = pMesh->mVertices[i].x;
			vertices[i].Position.y = pMesh->mVertices[i].y;
			vertices[i].Position.z = pMesh->mVertices[i].z;
			vertices[i].Position.w = 0.0f;

			if (pMesh->HasNormals())
			{
				vertices[i].Normal.x = pMesh->mNormals[i].x;
				vertices[i].Normal.y = pMesh->mNormals[i].y;
				vertices[i].Normal.z = pMesh->mNormals[i].z;
				vertices[i].Normal.w = 0.0f;
			}

			// There can be different sets of texture coords - unsure of purpose - only use the first
			if (pMesh->HasTextureCoords(0))
			{
				vertices[i].TexCoord.x = pMesh->mTextureCoords[0][i].x;
				vertices[i].TexCoord.y = pMesh->mTextureCoords[0][i].y;
			}

			if (pMesh->HasTangentsAndBitangents())
			{
				vertices[i].Tangent.x = pMesh->mTangents[i].x;
				vertices[i].Tangent.y = pMesh->mTangents[i].y;
				vertices[i].Tangent.z = pMesh->mTangents[i].z;
				vertices[i].Tangent.w = 0.0f;
			}
		}

		for (uint32 i = 0; i < pMesh->mNumFaces; i++)
		{
			indices[i * 3 + 0] = pMesh->mFaces[i].mIndices[0];
			indices[i * 3 + 1] = pMesh->mFaces[i].mIndices[1];
			indices[i * 3 + 2] = pMesh->mFaces[i].mIndices[2];
		}

		MeshRange meshRange = GeometryPool::UploadMesh(vertices, indices);
		return MeshAsset(assetID, meshRange);
	}

	SceneAsset::Node SceneAssetImporter::ImportNode(SceneAsset& sceneAsset, const aiScene* pScene, const aiNode* pNode)
	{
		SceneAsset::Node node;

		node.Name = pNode->mName.C_Str();

		glm::mat4 transform = ConvertAiMatToGLM(pNode->mTransformation);
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(transform, node.Scale, node.Orientation, node.Translation, skew, perspective);

		// These are indices into aiScene::mMeshes.
		for (uint32 i = 0; i < pNode->mNumMeshes; ++i)
		{
			uint32                 meshIndex     = pNode->mMeshes[i];
			uint32                 materialIndex = pScene->mMeshes[meshIndex]->mMaterialIndex;
			SceneAsset::Renderable renderable{
			    .MeshIndex     = meshIndex,
			    .MaterialIndex = materialIndex};

			node.Renderables.emplace_back(renderable);
		}

		for (uint32 i = 0; i < pNode->mNumChildren; ++i)
		{
			node.ChildrenIndices.emplace_back(sceneAsset.AddNode(ImportNode(sceneAsset, pScene, pNode->mChildren[i])));
		}

		return node;
	}
} // namespace Poly