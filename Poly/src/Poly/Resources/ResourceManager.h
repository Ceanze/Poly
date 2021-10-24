#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class Mesh;
	class Model;
	class Texture;
	class Material;

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
		ResourceManager() = default;
		~ResourceManager() = default;

		static void Release();

		/**
		 * Loads a texture from file and stages it to the GPU
		 * @param path - path of texture
		 * @param format - format of texture
		 * @return PolyID of texture
		 */
		static PolyID LoadTexture(const std::string& path, EFormat format);

		/**
		 * Loads a mesh from file
		 * @param path - path of mesh
		 * @return PolyID of mesh
		 */
		static PolyID LoadModel(const std::string& path);

		/**
		 * Loads a material from file
		 * @param path - path of material
		 * @return PolyID of material
		 */
		static PolyID LoadMaterial(const std::string& path);

		static Model*		GetModel(PolyID modelID);
		static Texture*		GetTexture(PolyID textureID);
		static Material*	GetMaterial(PolyID materialID);

		inline static bool IsResourceLoaded(const std::string& path) { return m_PathToEntry.contains(path); }

		static PolyID RegisterModel(const std::string& path, Ref<Model> pModel);
		static PolyID RegisterTexture(const std::string& path, Ref<Texture> pTexture);
		static PolyID RegisterMaterial(const std::string& path, Ref<Material> pMaterial);

	private:
		struct PolyIDEntry
		{
			PolyID			ID		= 0;
			ResourceType	Type	= ResourceType::NONE;
		};

		static std::vector<Ref<Texture>>	m_Textures;
		static std::vector<Ref<Model>>		m_Models;
		static std::vector<Ref<Material>>	m_Materials;

		static std::unordered_map<std::string, PolyIDEntry> m_PathToEntry;
	};
}