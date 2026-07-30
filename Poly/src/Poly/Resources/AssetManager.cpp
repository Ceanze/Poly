#include "AssetManager.h"

#include "AssetLoader.h"
#include "IOManager.h"
#include "Platform/API/Texture.h"
#include "Platform/API/TextureView.h"
#include "Poly/Core/RenderAPI.h"
#include "Poly/Model/Material.h"
#include "Poly/Model/Mesh.h"
#include "Poly/Model/Model.h"
#include "polypch.h"
#include "AssetImporter.h"

namespace Poly
{
	void AssetManager::Init()
	{
		RegisterDefaults();
		AssetImporter::LoadImports();
		RegisterImports();
	}

	void AssetManager::Release()
	{
		m_Models.clear();
		m_Textures.clear();
		m_Materials.clear();
		m_IDToHandle.clear();
	}

	PolyID AssetManager::ImportResource(const std::string& path, ResourceType type)
	{
		PolyID pathID = PolyID::None();
		if (type == ResourceType::MODEL || type == ResourceType::MESH)
		{
			pathID               = AssetImporter::ImportModel(path);
			m_IDToHandle[pathID] = {.Index = UINT32_MAX, .Type = ResourceType::MODEL, .IsLoaded = false};
		}
		else if (type == ResourceType::TEXTURE)
		{
			pathID               = AssetImporter::ImportTexture(path);
			m_IDToHandle[pathID] = {.Index = UINT32_MAX, .Type = ResourceType::TEXTURE, .IsLoaded = false};
		}
		else if (type == ResourceType::MATERIAL)
		{
			pathID               = AssetImporter::ImportMaterial(path);
			m_IDToHandle[pathID] = {.Index = UINT32_MAX, .Type = ResourceType::MATERIAL, .IsLoaded = false};
		}

		return pathID;
	}

	void AssetManager::LoadTexture(PolyID textureID, EFormat format)
	{
		if (!m_IDToHandle.contains(textureID))
		{
			POLY_CORE_WARN("Tried to load texture {}, but texture has never been imported", textureID);
			return;
		}

		ResourceHandle& handle = m_IDToHandle[textureID];

		if (handle.IsLoaded)
			return;

		// TODO: As for now all calls to AssetLoader are done with Absolute Paths - but we save in Relative Paths.
		//		 Either make sure this is clear, or have internal functions to check if path is Absolute or Relative
		//		 to handle it.
		Ref<Texture> pTex = AssetLoader::LoadTexture(IOManager::GetAssetsFolder() + handle.Path, format);

		// TODO: Grab necessary data from texture
		Poly::TextureViewDesc textureViewDesc = {
		    .pTexture        = pTex.get(),
		    .ImageViewType   = Poly::EImageViewType::TYPE_2D,
		    .Format          = format,
		    .ImageViewFlag   = Poly::FImageViewFlag::COLOR,
		    .MipLevel        = 0,
		    .MipLevelCount   = 1,
		    .ArrayLayer      = 0,
		    .ArrayLayerCount = 1,
		};
		Ref<TextureView> pTextureView = RenderAPI::CreateTextureView(&textureViewDesc);

		uint32 index = static_cast<uint32>(m_Textures.size());
		m_Textures.push_back({.pTexture = pTex, .pTextureView = pTextureView});

		handle.Index    = index;
		handle.IsLoaded = true;
	}

	PolyID AssetManager::ImportAndLoadTexture(const std::string& path, EFormat format)
	{
		std::string relativePath = path;

		PolyID pathID = AssetImporter::ImportTexture(path);

		if (!m_IDToHandle.contains(pathID))
		{
			ResourceHandle handle = {};
			handle.Path           = relativePath;
			handle.Type           = ResourceType::TEXTURE;
			m_IDToHandle[pathID]  = handle;
		}

		LoadTexture(pathID, format);

		return pathID;
	}

	void AssetManager::LoadModel(PolyID modelID, Entity root)
	{
		if (!m_IDToHandle.contains(modelID))
		{
			POLY_CORE_WARN("Tried to load model {}, but texture has never been imported", modelID);
			return;
		}

		ResourceHandle& handle = m_IDToHandle[modelID];

		if (handle.IsLoaded)
			return;

		Ref<Model> pModel = AssetLoader::LoadModel(handle.Path, root);
		pModel->SetModelID(modelID);
		uint32 index = static_cast<uint32>(m_Models.size());
		m_Models.push_back(pModel);

		handle.Index    = index;
		handle.IsLoaded = true;
	}

	PolyID AssetManager::ImportAndLoadModel(const std::string& path, Entity root)
	{
		std::string relativePath = path;

		PolyID pathID = AssetImporter::ImportModel(path);

		if (!m_IDToHandle.contains(pathID))
		{
			ResourceHandle handle = {};
			handle.Path           = relativePath;
			handle.Type           = ResourceType::MODEL;
			m_IDToHandle[pathID]  = handle;
		}

		LoadModel(pathID, root);

		return pathID;
	}

	void AssetManager::LoadMaterial(PolyID materialID)
	{
		if (!m_IDToHandle.contains(materialID))
		{
			POLY_CORE_WARN("Tried to load material {}, but texture has never been imported", materialID);
			return;
		}

		ResourceHandle& handle = m_IDToHandle[materialID];

		if (handle.IsLoaded)
			return;

		Ref<Material> pMaterial = AssetLoader::LoadMaterial(handle.Path);
		uint32        index     = static_cast<uint32>(m_Materials.size());
		m_Materials.push_back(pMaterial);

		handle.Index    = index;
		handle.IsLoaded = true;
	}

	PolyID AssetManager::ImportAndLoadMaterial(const std::string& path)
	{
		std::string relativePath = IOManager::GetAssetsFolder() + path;

		PolyID pathID = AssetImporter::ImportMaterial(path);

		if (!m_IDToHandle.contains(pathID))
		{
			ResourceHandle handle = {};
			handle.Path           = relativePath;
			handle.Type           = ResourceType::MATERIAL;
			m_IDToHandle[pathID]  = handle;
		}

		LoadMaterial(pathID);

		return pathID;
	}

	Mesh* AssetManager::GetMesh(PolyID modelID, uint32 meshIndex)
	{
		if (!HasCorrectResource(modelID, ResourceType::MODEL))
		{
			POLY_CORE_WARN("Cannot get model {}. It is either wrong ID, not loaded and/or not imported", modelID);
			return nullptr;
		}

		uint32 index = m_IDToHandle[modelID].Index;

		if (index < m_Models.size())
			return m_Models[index]->GetMesh(meshIndex);

		POLY_CORE_WARN("Tried to get mesh with ID {}, but ID was out of range", index);
		return nullptr;
	}

	Model* AssetManager::GetModel(PolyID modelID)
	{
		if (!HasCorrectResource(modelID, ResourceType::MODEL))
		{
			POLY_CORE_WARN("Cannot get model {}. It is either wrong ID, not loaded and/or not imported", modelID);
			return nullptr;
		}

		uint32 index = m_IDToHandle[modelID].Index;

		if (index < m_Models.size())
			return m_Models[index].get();

		POLY_CORE_WARN("Tried to get mesh with ID {}, but ID was out of range", index);
		return nullptr;
	}

	Texture* AssetManager::GetTexture(PolyID textureID)
	{
		if (!HasCorrectResource(textureID, ResourceType::TEXTURE))
		{
			POLY_CORE_WARN("Cannot get texture {}. It is either wrong ID, not loaded and/or not imported", textureID);
			return nullptr;
		}

		uint32 index = m_IDToHandle[textureID].Index;

		if (index < m_Textures.size())
			return m_Textures[index].pTexture.get();

		POLY_CORE_WARN("Tried to get texture with ID {}, but ID was out of range", index);
		return nullptr;
	}

	TextureView* AssetManager::GetTextureView(PolyID textureViewID)
	{
		// Texture and texture view is saved under same ID
		if (!HasCorrectResource(textureViewID, ResourceType::TEXTURE))
		{
			POLY_CORE_WARN("Cannot get textureview {}. It is either wrong ID, not loaded and/or not imported", textureViewID);
			return nullptr;
		}

		uint32 index = m_IDToHandle[textureViewID].Index;

		if (index < m_Textures.size())
			return m_Textures[index].pTextureView.get();

		POLY_CORE_WARN("Tried to get texture view with ID {}, but ID was out of range", index);
		return nullptr;
	}

	ManagedTexture AssetManager::GetManagedTexture(PolyID polyTextureID)
	{
		// Texture, texture view, and managed texture is saved under same ID
		if (polyTextureID > 0 && !HasCorrectResource(polyTextureID, ResourceType::TEXTURE))
		{
			POLY_CORE_WARN("Cannot get managed texture {}. It is either wrong ID, not loaded and/or not imported", polyTextureID);
			return {};
		}

		uint32 index = m_IDToHandle[polyTextureID].Index;

		if (index < m_Textures.size())
			return m_Textures[index];

		POLY_CORE_WARN("Tried to get poly texture with ID {}, but ID was out of range", index);
		return {};
	}

	Material* AssetManager::GetMaterial(PolyID materialID)
	{
		if (!HasCorrectResource(materialID, ResourceType::TEXTURE))
		{
			POLY_CORE_WARN("Cannot get material {}. It is either wrong ID, not loaded and/or not imported", materialID);
			return {};
		}

		uint32 index = m_IDToHandle[materialID].Index;

		if (index < m_Materials.size())
			return m_Materials[index].get();

		POLY_CORE_WARN("Tried to get material with ID {}, but ID was out of range, returning default material", index);
		return m_Materials[DEFAULT_MATERIAL_ID].get();
	}

	Material* AssetManager::GetMaterial(PolyID modelID, uint32 meshIndex)
	{
		if (!HasCorrectResource(modelID, ResourceType::MODEL))
		{
			POLY_CORE_WARN("Cannot get model {}. It is either wrong ID, not loaded and/or not imported", modelID);
			return nullptr;
		}

		uint32 index = m_IDToHandle[modelID].Index;

		if (index < m_Models.size())
			return m_Models[index]->GetMaterial(meshIndex);

		POLY_CORE_WARN("Tried to get material with ID {}, but ID was out of range", index);
		return nullptr;
	}

	bool AssetManager::IsResourceImported(const std::string& path)
	{
		return AssetImporter::IsImported(path);
	}

	bool AssetManager::IsResourceLoaded(PolyID id)
	{
		return m_IDToHandle.contains(id) && m_IDToHandle[id].IsLoaded;
	}

	PolyID AssetManager::GetPolyIDFromPath(const std::string& path)
	{
		std::string relativePath = path;
		return AssetImporter::GetPathID(relativePath);
	}

	void AssetManager::RegisterDefaults()
	{
		RegisterDefaultMaterial();

		ResourceHandle handle             = {};
		handle.Index                      = 0;
		handle.IsLoaded                   = true;
		m_IDToHandle[DEFAULT_MATERIAL_ID] = handle;
	}

	void AssetManager::RegisterDefaultMaterial()
	{
		Ref<Material> pMaterial = CreateRef<Material>();

		m_Materials.push_back(pMaterial);

		uint32       width    = 1;
		uint32       height   = 1;
		uint32       channels = 4;
		byte         data[4]  = {255, 255, 255, 255};
		Ref<Texture> pTexture = AssetLoader::LoadTextureFromMemory(&data, width, height, channels, EFormat::R8G8B8A8_UNORM);

		Poly::TextureViewDesc textureViewDesc = {
		    .pTexture        = pTexture.get(),
		    .ImageViewType   = Poly::EImageViewType::TYPE_2D,
		    .Format          = EFormat::R8G8B8A8_UNORM,
		    .ImageViewFlag   = Poly::FImageViewFlag::COLOR,
		    .MipLevel        = 0,
		    .MipLevelCount   = 1,
		    .ArrayLayer      = 0,
		    .ArrayLayerCount = 1,
		};
		Ref<TextureView> pTextureView = RenderAPI::CreateTextureView(&textureViewDesc);

		m_Textures.push_back({pTexture, pTextureView});

		pMaterial->SetTexture(Material::Type::ALBEDO, pTexture.get());
		pMaterial->SetTextureView(Material::Type::ALBEDO, pTextureView.get());

		MaterialValues matVals = {};
		pMaterial->SetMaterialValues(matVals);
	}

	void AssetManager::RegisterImports()
	{
		const auto& imports = AssetImporter::GetImports();
		for (const auto& [path, importedResource] : imports)
		{
			if (!m_IDToHandle.contains(importedResource.ResourceID))
			{
				ResourceHandle handle                     = {};
				handle.Path                               = path;
				handle.Type                               = importedResource.Type;
				handle.IsLoaded                           = false;
				handle.Index                              = UINT32_MAX;
				m_IDToHandle[importedResource.ResourceID] = handle;
			}
		}
	}

	bool AssetManager::HasCorrectResource(PolyID id, ResourceType type)
	{
		if (!m_IDToHandle.contains(id))
			return false;

		return m_IDToHandle[id].Type == type && m_IDToHandle[id].IsLoaded;
	}

	std::vector<ManagedTexture> AssetManager::m_Textures;
	std::vector<Ref<Model>>     AssetManager::m_Models;
	std::vector<Ref<Material>>  AssetManager::m_Materials;

	std::unordered_map<PolyID, AssetManager::ResourceHandle> AssetManager::m_IDToHandle;
} // namespace Poly
