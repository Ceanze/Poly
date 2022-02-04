#pragma once

#include "Poly/Scene/Entity.h"
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
		inline static const PolyID DEFAULT_MATERIAL_ID = PolyID::None();
		inline static const PolyID DEFAULT_TEXTURE_ID = PolyID::None();

	public:
		ResourceManager() = default;
		~ResourceManager() = default;

		static void Init();
		static void Release();

		/**
		 * Imports a resource which links a path to a newly generated PolyID which is used for other resource functions
		 * @param path - path of the resource relative to assets/ folder
		 * @param type - resource type, if none provided it will be deduced by file extension
		 * @return PolyID of the now imported resource
		 */
		static PolyID ImportResource(const std::string& path, ResourceType type = ResourceType::NONE);

		/**
		 * Loads a texture from file and stages it to the GPU
		 * @param path - path of texture
		 * @param format - format of texture
		 */
		static void LoadTexture(PolyID textureID, EFormat format);

		/**
		 * Shorthand for calling Import and then Load
		 * @param path - path of texture
		 * @param format - format of texture
		 * @return PolyID of texture
		 */
		static PolyID ImportAndLoadTexture(const std::string& path, EFormat format);

		/**
		 * Loads model from file and creates a hierarchy with the entity as root
		 * @param path - path of model
		 * @param root - root entity of the mesh hierarhcy
		 */
		static void LoadModel(PolyID modelID, Entity root);

		/**
		 * Loads model from file and creates a hierarchy with the entity as root
		 * @param path - path of model
		 * @param root - root entity of the mesh hierarhcy
		 * @return PolyID of model
		 */
		static PolyID ImportAndLoadModel(const std::string& path, Entity root);

		/**
		 * Loads a material from file
		 * @param path - path of material
		 */
		static void LoadMaterial(PolyID materialID);

		/**
		 * Loads a material from file
		 * @param path - path of material
		 * @return PolyID of material
		 */
		static PolyID ImportAndLoadMaterial(const std::string& path);

		/**
		 * Unloads the resource from memory
		 * @param id - PolyID of the resource
		 */
		static void UnloadResource(PolyID id);

		static Mesh*			GetMesh(PolyID modelID, uint32 meshIndex);
		static Model*			GetModel(PolyID modelID);
		static Texture*			GetTexture(PolyID textureID);
		static TextureView*		GetTextureView(PolyID textureViewID);
		static ManagedTexture	GetManagedTexture(PolyID polyTextureID);
		static Material*		GetMaterial(PolyID materialID);
		static Material*		GetMaterial(PolyID modelID, uint32 meshIndex);

		static bool IsResourceImported(const std::string& path);
		static bool IsResourceLoaded(PolyID id);
		static PolyID GetPolyIDFromPath(const std::string& path);

	private:
		static void RegisterDefaults();
		static void RegisterDefaultMaterial();
		static bool HasCorrectResource(PolyID id, ResourceType type);

		struct ResourceHandle
		{
			uint32			Index		= UINT32_MAX;
			ResourceType	Type		= ResourceType::NONE;
			bool			IsLoaded	= false;
			std::string		Path		= "";
		};

		static std::vector<ManagedTexture>	m_Textures;
		static std::vector<Ref<Model>>		m_Models;
		static std::vector<Ref<Material>>	m_Materials;

		static std::unordered_map<PolyID, ResourceHandle> m_IDToHandle;
	};
}