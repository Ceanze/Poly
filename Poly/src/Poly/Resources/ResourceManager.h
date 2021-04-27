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
		static PolyID LoadMesh(const std::string& path);

		/**
		 * Loads a material from file
		 * @param path - path of material
		 * @return PolyID of material
		 */
		static PolyID LoadMaterial(const std::string& path);

		static Material*	GetMaterial(PolyID materialID);
		static Mesh*		GetMesh(PolyID meshID);
		static Texture*		GetTexture(PolyID textureID);

	private:
		struct PolyIDEntry
		{
			PolyID			ID		= 0;
			ResourceType	Type	= ResourceType::NONE;
		};

		static std::vector<Ref<Texture>>	m_Textures;
		static std::vector<Mesh>			m_Meshes;
		static std::vector<Material>		m_Materials;
		static std::vector<Model>			m_Models;

		static std::unordered_map<std::string, PolyIDEntry> m_PathToEntry;
	};
}