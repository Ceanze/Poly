#include "polypch.h"
#include "ResourceManager.h"

#include "IOManager.h"
#include "ResourceLoader.h"
#include "Poly/Model/Mesh.h"
#include "Poly/Model/Model.h"
#include "Poly/Model/Material.h"
#include "Poly/Core/RenderAPI.h"
#include "Platform/API/Texture.h"
#include "Platform/API/TextureView.h"

namespace Poly
{
	void ResourceManager::Init()
	{
		RegisterDefaultMaterial();
	}

	void ResourceManager::Release()
	{
		m_Models.clear();
		m_Textures.clear();
		m_Materials.clear();
		m_PathToEntry.clear();
	}

	PolyID ResourceManager::LoadTexture(const std::string& path, EFormat format)
	{
		std::string relativePath = IOManager::GetAssetsFolder() + path;
		if (m_PathToEntry.contains(relativePath))
		{
			PolyIDEntry entry = m_PathToEntry[relativePath];
			if (entry.Type == ResourceType::TEXTURE)
				return entry.ID;
			else
			{
				POLY_CORE_WARN("Tried to load texture with path {}, but path has been used before as a different type", relativePath);
				return POLY_ID_UNKNOWN;
			}
		}

		Ref<Texture> pTex = ResourceLoader::LoadTexture(relativePath, format);

		// TODO: Grab necessary data from texture
		Poly::TextureViewDesc textureViewDesc = {
			.pTexture			= pTex.get(),
			.ImageViewType		= Poly::EImageViewType::TYPE_2D,
			.Format				= format,
			.ImageViewFlag		= Poly::FImageViewFlag::COLOR,
			.MipLevel			= 0,
			.MipLevelCount		= 1,
			.ArrayLayer			= 0,
			.ArrayLayerCount	= 1,
		};
		Ref<TextureView> pTextureView = RenderAPI::CreateTextureView(&textureViewDesc);

		PolyID id = m_Textures.size();
		m_Textures.push_back({ .pTexture = pTex, .pTextureView = pTextureView });
		m_PathToEntry[relativePath] = { .ID = id, .Type = ResourceType::TEXTURE };
		return id;
	}

	PolyID ResourceManager::LoadModel(const std::string& path)
	{
		std::string relativePath = IOManager::GetAssetsFolder() + path;
		if (m_PathToEntry.contains(relativePath))
		{
			PolyIDEntry entry = m_PathToEntry[relativePath];
			if (entry.Type == ResourceType::MODEL)
				return entry.ID;
			else
			{
				POLY_CORE_WARN("Tried to load model with path {}, but path has been used before as a different type", relativePath);
				return POLY_ID_UNKNOWN;
			}
		}

		Ref<Model> pModel = ResourceLoader::LoadModel(relativePath);
		PolyID id = m_Models.size();
		m_Models.push_back(pModel);
		m_PathToEntry[relativePath] = { .ID = id, .Type = ResourceType::MODEL };
		return id;
	}

	PolyID ResourceManager::LoadMaterial(const std::string& path)
	{
		std::string relativePath = IOManager::GetAssetsFolder() + path;
		if (m_PathToEntry.contains(relativePath))
		{
			PolyIDEntry entry = m_PathToEntry[relativePath];
			if (entry.Type == ResourceType::MATERIAL)
				return entry.ID;
			else
			{
				POLY_CORE_WARN("Tried to load material with path {}, but path has been used before as a different type", relativePath);
				return POLY_ID_UNKNOWN;
			}
		}

		Ref<Material> pMaterial = ResourceLoader::LoadMaterial(relativePath);
		PolyID id = m_Materials.size();
		m_Materials.push_back(pMaterial);
		m_PathToEntry[relativePath] = { .ID = id, .Type = ResourceType::MATERIAL };
		return id;
	}

	PolyID ResourceManager::GetPolyIDFromPath(const std::string& path)
	{
		std::string relativePath = IOManager::GetAssetsFolder() + path;
		if (IsResourceLoaded(relativePath))
			return m_PathToEntry[relativePath].ID;
		else
		{
			POLY_CORE_WARN("Tried to get a PolyID from path '{}' when no resource was loaded with that path", relativePath);
			return POLY_ID_UNKNOWN;
		}
	}

	PolyID ResourceManager::RegisterModel(const std::string& path, Ref<Model> pModel)
	{
		std::string relativePath = IOManager::GetAssetsFolder() + path;
		if (m_PathToEntry.contains(relativePath))
		{
			POLY_CORE_WARN("A resource is already registered to path {} with ID {} and type {}!", relativePath, m_PathToEntry[relativePath].ID, m_PathToEntry[relativePath].Type);
			return POLY_ID_UNKNOWN;
		}

		PolyIDEntry entry = {};
		entry.Type	= ResourceType::MODEL;
		entry.ID	= m_Models.size();

		m_Models.push_back(pModel);
		m_PathToEntry[relativePath] = entry;

		return entry.ID;
	}

	PolyID ResourceManager::RegisterPolyTexture(const std::string& path, Ref<Texture> pTexture, Ref<TextureView> pTextureView)
	{
		std::string relativePath = IOManager::GetAssetsFolder() + path;
		if (m_PathToEntry.contains(relativePath))
		{
			POLY_CORE_WARN("A resource is already registered to path {} with ID {} and type {}!", relativePath, m_PathToEntry[relativePath].ID, m_PathToEntry[relativePath].Type);
			return POLY_ID_UNKNOWN;
		}

		PolyIDEntry entry = {};
		entry.Type	= ResourceType::TEXTURE;
		entry.ID	= m_Textures.size();

		m_Textures.push_back({ .pTexture = pTexture, .pTextureView = pTextureView });
		m_PathToEntry[relativePath] = entry;

		return entry.ID;
	}

	PolyID ResourceManager::RegisterMaterial(const std::string& path, Ref<Material> pMaterial)
	{
		std::string relativePath = IOManager::GetAssetsFolder() + path;
		if (m_PathToEntry.contains(relativePath))
		{
			POLY_CORE_WARN("A resource is already registered to path {} with ID {} and type {}!", relativePath, m_PathToEntry[relativePath].ID, m_PathToEntry[relativePath].Type);
			return POLY_ID_UNKNOWN;
		}

		if (!pMaterial)
		{
			POLY_CORE_WARN("Can't register a null material! Path: {}", relativePath);
			return POLY_ID_UNKNOWN;
		}

		PolyIDEntry entry = {};
		entry.Type	= ResourceType::MATERIAL;
		entry.ID	= m_Materials.size();

		m_Materials.push_back(pMaterial);
		m_PathToEntry[relativePath] = entry;

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
			return m_Textures[textureID].pTexture.get();

		POLY_CORE_WARN("Tried to get texture with ID {}, but ID was out of range", textureID);
		return nullptr;
	}

	TextureView* ResourceManager::GetTextureView(PolyID textureViewID)
	{
		if (textureViewID < m_Textures.size())
			return m_Textures[textureViewID].pTextureView.get();

		POLY_CORE_WARN("Tried to get texture view with ID {}, but ID was out of range", textureViewID);
		return nullptr;
	}

	ManagedTexture ResourceManager::GetManagedTexture(PolyID polyTextureID)
	{
		if (polyTextureID < m_Textures.size())
			return m_Textures[polyTextureID];

		POLY_CORE_WARN("Tried to get poly texture with ID {}, but ID was out of range", polyTextureID);
		return {};
	}

	Material* ResourceManager::GetMaterial(PolyID materialID)
	{
		if (materialID < m_Materials.size())
			return m_Materials[materialID].get();

		POLY_CORE_WARN("Tried to get material with ID {}, but ID was out of range, returning default material", materialID);
		return m_Materials[DEFAULT_MATERIAL_ID].get();
	}

	void ResourceManager::RegisterDefaultMaterial()
	{
		Ref<Material> pMaterial = CreateRef<Material>();

		m_Materials.push_back(pMaterial);

		uint32 width	= 1;
		uint32 height	= 1;
		uint32 channels	= 4;
		byte data[4] = { 255, 255, 255, 255 };
		Ref<Texture> pTexture = ResourceLoader::LoadTextureFromMemory(&data, width, height, channels, EFormat::R8G8B8A8_UNORM);

		Poly::TextureViewDesc textureViewDesc = {
			.pTexture			= pTexture.get(),
			.ImageViewType		= Poly::EImageViewType::TYPE_2D,
			.Format				= EFormat::R8G8B8A8_UNORM,
			.ImageViewFlag		= Poly::FImageViewFlag::COLOR,
			.MipLevel			= 0,
			.MipLevelCount		= 1,
			.ArrayLayer			= 0,
			.ArrayLayerCount	= 1,
		};
		Ref<TextureView> pTextureView = RenderAPI::CreateTextureView(&textureViewDesc);

		m_Textures.push_back({pTexture, pTextureView});

		pMaterial->SetTexture(Material::Type::ALBEDO, pTexture.get());
		pMaterial->SetTextureView(Material::Type::ALBEDO, pTextureView.get());

		MaterialValues matVals = {};
		pMaterial->SetMaterialValues(matVals);
	}


	std::vector<ManagedTexture>	ResourceManager::m_Textures;
	std::vector<Ref<Model>>		ResourceManager::m_Models;
	std::vector<Ref<Mesh>>		ResourceManager::m_Meshes;
	std::vector<Ref<Material>>	ResourceManager::m_Materials;

	std::unordered_map<std::string, ResourceManager::PolyIDEntry> ResourceManager::m_PathToEntry;
}