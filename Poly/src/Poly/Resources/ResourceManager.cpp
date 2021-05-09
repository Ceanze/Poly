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
			return m_Materials[materialID].get();

		POLY_CORE_WARN("Tried to get material with ID {}, but ID was out of range", materialID);
		return nullptr;
	}

	PolyID ResourceManager::RegisterModel(const std::string& path, Ref<Model> pModel)
	{
		if (m_PathToEntry.contains(path))
		{
			POLY_CORE_WARN("A resource is already registered to path {} with ID {} and type {}!", path, m_PathToEntry[path].ID, m_PathToEntry[path].Type);
			return UINT32_MAX;
		}

		PolyIDEntry entry = {};
		entry.Type	= ResourceType::MODEL;
		entry.ID	= m_Models.size();

		m_Models.push_back(pModel);
		m_PathToEntry[path] = entry;

		return entry.ID;
	}

	PolyID ResourceManager::RegisterTexture(const std::string& path, Ref<Texture> pTexture)
	{
		if (m_PathToEntry.contains(path))
		{
			POLY_CORE_WARN("A resource is already registered to path {} with ID {} and type {}!", path, m_PathToEntry[path].ID, m_PathToEntry[path].Type);
			return UINT32_MAX;
		}

		PolyIDEntry entry = {};
		entry.Type	= ResourceType::TEXTURE;
		entry.ID	= m_Textures.size();

		m_Textures.push_back(pTexture);
		m_PathToEntry[path] = entry;

		return entry.ID;
	}

	PolyID ResourceManager::RegisterMaterial(const std::string& path, Ref<Material> pMaterial)
	{
		if (m_PathToEntry.contains(path))
		{
			POLY_CORE_WARN("A resource is already registered to path {} with ID {} and type {}!", path, m_PathToEntry[path].ID, m_PathToEntry[path].Type);
			return UINT32_MAX;
		}

		PolyIDEntry entry = {};
		entry.Type	= ResourceType::MATERIAL;
		entry.ID	= m_Materials.size();

		m_Materials.push_back(pMaterial);
		m_PathToEntry[path] = entry;

		return entry.ID;
	}

	Model* ResourceManager::GetModel(PolyID modelID)
	{
		if (modelID < m_Models.size())
			return m_Models[modelID].get();

		POLY_CORE_WARN("Tried to get model with ID {}, but ID was out of range", modelID);
		return nullptr;
	}

	Texture* ResourceManager::GetTexture(PolyID textureID)
	{
		if (textureID < m_Textures.size())
			return m_Textures[textureID].get();

		POLY_CORE_WARN("Tried to get texture with ID {}, but ID was out of range", textureID);
		return nullptr;
	}

	std::vector<Ref<Texture>>	ResourceManager::m_Textures;
	std::vector<Ref<Model>>		ResourceManager::m_Models;
	std::vector<Ref<Material>>	ResourceManager::m_Materials;

	std::unordered_map<std::string, ResourceManager::PolyIDEntry> ResourceManager::m_PathToEntry;
}