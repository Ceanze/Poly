#include "polypch.h"
#include "ResourceManager.h"

#include "ResourceLoader.h"
#include "Poly/Model/Mesh.h"
#include "Poly/Model/Model.h"
#include "Poly/Model/Material.h"
#include "Platform/API/Texture.h"

namespace Poly
{
	PolyID ResourceManager::LoadTexture(const std::string& path, EFormat format)
	{
		if (m_PathToEntry.contains(path))
		{
			PolyIDEntry entry = m_PathToEntry[path];
			if (entry.Type == ResourceType::TEXTURE)
				return entry.ID;
			else
			{
				POLY_CORE_WARN("Tried to load texture with path {}, but path has been used before as a different type", path);
				return UINT32_MAX;
			}
		}

		Ref<Texture> pTex = ResourceLoader::LoadTexture(path, format);
		PolyID id = m_Textures.size();
		m_Textures.push_back(pTex);
		m_PathToEntry[path] = { .ID = id, .Type = ResourceType::TEXTURE };
		return id;
	}

	PolyID ResourceManager::LoadModel(const std::string& path)
	{
		if (m_PathToEntry.contains(path))
		{
			PolyIDEntry entry = m_PathToEntry[path];
			if (entry.Type == ResourceType::MODEL)
				return entry.ID;
			else
			{
				POLY_CORE_WARN("Tried to load model with path {}, but path has been used before as a different type", path);
				return UINT32_MAX;
			}
		}
	}

	PolyID ResourceManager::LoadMesh(const std::string& path)
	{
		if (m_PathToEntry.contains(path))
		{
			PolyIDEntry entry = m_PathToEntry[path];
			if (entry.Type == ResourceType::MESH)
				return entry.ID;
			else
			{
				POLY_CORE_WARN("Tried to load mesh with path {}, but path has been used before as a different type", path);
				return UINT32_MAX;
			}
		}
	}

	PolyID ResourceManager::LoadMaterial(const std::string& path)
	{
		if (m_PathToEntry.contains(path))
		{
			PolyIDEntry entry = m_PathToEntry[path];
			if (entry.Type == ResourceType::MATERIAL)
				return entry.ID;
			else
			{
				POLY_CORE_WARN("Tried to load material with path {}, but path has been used before as a different type", path);
				return UINT32_MAX;
			}
		}
	}

	Material* ResourceManager::GetMaterial(PolyID materialID)
	{
		if (materialID < m_Materials.size())
			return &m_Materials[materialID];

		POLY_CORE_WARN("Tried to get material with ID {}, but ID was out of range", materialID);
		return nullptr;
	}

	Model* ResourceManager::GetModel(PolyID modelID)
	{
		if (modelID < m_Models.size())
			return &m_Models[modelID];

		POLY_CORE_WARN("Tried to get model with ID {}, but ID was out of range", modelID);
		return nullptr;
	}

	Mesh* ResourceManager::GetMesh(PolyID meshID)
	{
		if (meshID < m_Meshes.size())
			return &m_Meshes[meshID];

		POLY_CORE_WARN("Tried to get material with ID {}, but ID was out of range", meshID);
		return nullptr;
	}

	Texture* ResourceManager::GetTexture(PolyID textureID)
	{
		if (textureID < m_Textures.size())
			return m_Textures[textureID].get();

		POLY_CORE_WARN("Tried to get material with ID {}, but ID was out of range", textureID);
		return nullptr;
	}

	std::vector<Ref<Texture>>	ResourceManager::m_Textures;
	std::vector<Mesh>			ResourceManager::m_Meshes;
	std::vector<Material>		ResourceManager::m_Materials;
	std::vector<Model>			ResourceManager::m_Models;

	std::unordered_map<std::string, ResourceManager::PolyIDEntry> ResourceManager::m_PathToEntry;
}