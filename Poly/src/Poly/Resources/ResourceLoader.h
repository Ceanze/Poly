#pragma once

#include "Poly/Model/Material.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Poly/Scene/Entity.h"

#include <vector>

struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;

namespace Poly
{
	class Mesh;
	class Model;
	class Shader;
	class Buffer;
	class Texture;
	class CommandPool;
	class CommandBuffer;
	class BinarySemaphore;

	struct MeshMaterialRefPair
	{
		Ref<Mesh>     pMesh;
		Ref<Material> pMaterial;
	};

	class ResourceLoader
	{
	public:
		ResourceLoader()  = default;
		~ResourceLoader() = default;

		static void Init();
		static void Release();

		static std::vector<byte> LoadShader(std::string_view path, FShaderStage shaderStage);

		static std::vector<byte> LoadRawImage(const std::string& path);

		static Ref<Texture> LoadTexture(const std::string& path, EFormat format);

		static Ref<Texture> LoadTextureFromMemory(void* data, uint32 width, uint32 height, uint32 channels, EFormat format);

		static Ref<Model> LoadModel(const std::string& path, Entity root);

		static Ref<Material> LoadMaterial(const std::string& path);

	private:
		static void          ProcessNode(aiNode* pNode, const aiScene* pScene, const std::string& folder, Model* pModel, Entity parent);
		static Ref<Mesh>     ProcessMesh(aiMesh* pMesh, const aiScene* pScene, Model* pModel, uint32 index);
		static Ref<Material> ProcessMaterial(aiMaterial* pMaterial, const aiScene* pScene, Model* pModel, uint32 index, const std::string& folder);
		static void          TransferDataToGPU(const void* data, uint64 size, uint32 count, Ref<Buffer> pDestinationBuffer);
		static glm::mat4     ConvertAiMatToGLM(const void* pMat);

		inline static bool s_GLSLInit = false;
		;

		inline static Ref<CommandPool>     s_TransferCommandPool;
		inline static CommandBuffer*       s_TransferCommandBuffer = nullptr;
		inline static Ref<CommandPool>     s_GraphicsCommandPool;
		inline static CommandBuffer*       s_GraphicsCommandBuffer = nullptr;
		inline static Ref<BinarySemaphore> s_Semaphore;
	};
} // namespace Poly
