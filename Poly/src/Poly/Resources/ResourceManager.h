#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class Mesh;
	class Model;
	class Texture;
	class Material;
	class TextureView;

	struct ManagedTexture
	{
		Ref<Texture>		pTexture		= nullptr;
		Ref<TextureView>	pTextureView	= nullptr;
	};

	class ResourceManager
	{
	private:
		enum class ResourceType
		{
			NONE		= 0,
			TEXTURE		= 1,
			MODEL		= 2,
			MESH		= 3,
			MATERIAL	= 4
		};

	public:
		static const PolyID DEFAULT_MATERIAL_ID = 0;
		static const PolyID DEFAULT_TEXTURE_ID = 0;

	public:
		ResourceManager() = default;
		~ResourceManager() = default;

		static void Init();
		static void Release();

		/**
		 * Loads a texture from file and stages it to the GPU
		 * @param path - path of texture
		 * @param format - format of texture
		 * @return PolyID of texture
		 */
		static PolyID LoadTexture(const std::string& path, EFormat format);

		/**
		 * Loads a model from file
		 * NOTE: This is only used when not having ECS
		 * @param path - path of model
		 * @return PolyID of model
		 */
		static PolyID LoadModel(const std::string& path);

		/**
		 * Loads a mesh from a path
		 * @param path - path of mesh
		 * @return PolyID of mesh
		 */
		// static PolyID LoadMesh(const std::string& path);

		/**
		 * Loads a material from file
		 * @param path - path of material
		 * @return PolyID of material
		 */
		static PolyID LoadMaterial(const std::string& path);

		static Model*			GetModel(PolyID modelID);
		static Texture*			GetTexture(PolyID textureID);
		static TextureView*		GetTextureView(PolyID textureViewID);
		static ManagedTexture	GetManagedTexture(PolyID polyTextureID);
		static Material*		GetMaterial(PolyID materialID);

		inline static bool IsResourceLoaded(const std::string& path) { return m_PathToEntry.contains(path); }
		static PolyID GetPolyIDFromPath(const std::string& path);

		static PolyID RegisterModel(const std::string& path, Ref<Model> pModel);
		static PolyID RegisterPolyTexture(const std::string& path, Ref<Texture> pTexture, Ref<TextureView> pTextureView);
		static PolyID RegisterMaterial(const std::string& path, Ref<Material> pMaterial);

	private:
		static void RegisterDefaultMaterial();

		struct PolyIDEntry
		{
			PolyID			ID		= 0;
			ResourceType	Type	= ResourceType::NONE;
		};

		static std::vector<ManagedTexture>	m_Textures;
		static std::vector<Ref<Model>>		m_Models;
		static std::vector<Ref<Mesh>>		m_Meshes;
		static std::vector<Ref<Material>>	m_Materials;

		static std::unordered_map<std::string, PolyIDEntry> m_PathToEntry;
	};
}