#pragma once

#include <vector>
#include <string>

#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Poly/Model/Material.h"
#include "Poly/Scene/Entity.h"

struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;

enum aiTextureType;

namespace Poly
{
	class Mesh;
	class Model;
	class Shader;
	class Buffer;
	class Texture;
	class Semaphore;
	class CommandPool;
	class CommandBuffer;

	struct MeshMaterialRefPair
	{
		Ref<Mesh> pMesh;
		Ref<Material> pMaterial;
	};

	class ResourceLoader
	{
	public:
		ResourceLoader() = default;
		~ResourceLoader() = default;

		static void Init();
		static void Release();

		static Ref<Shader> LoadShader(const std::string& path, FShaderStage shaderStage);

		static std::vector<byte> LoadRawImage(const std::string& path);

		static Ref<Texture> LoadTexture(const std::string& path, EFormat format);

		static Ref<Texture> LoadTextureFromMemory(void* data, uint32 width, uint32 height, uint32 channels, EFormat format);

		static Ref<Model> LoadModel(const std::string& path, Entity root);

		static Ref<Material> LoadMaterial(const std::string& path);

	private:
		static void ProcessNode(aiNode* pNode, const aiScene* pScene, const std::string& folder, Model* pModel, Entity parent);
		static Ref<Mesh> ProcessMesh(aiMesh* pMesh, const aiScene* pScene, Model* pModel, uint32 index);
		static Ref<Material> ProcessMaterial(aiMaterial* pMaterial, const aiScene* pScene, Model* pModel, uint32 index, const std::string& folder);
		static void TransferDataToGPU(const void* data, uint32 size, uint32 count, Ref<Buffer> pDestinationBuffer);
		static glm::mat4 ConvertAiMatToGLM(const void* pMat);
		static void LoadAssimpMaterial(aiMaterial* pMaterial, aiTextureType type, uint32 index, const Ref<Material>& pPolyMaterial, const std::string& folder);
		static Material::Type ConvertTextureType(aiTextureType aiType);

		static bool s_GLSLInit;

		static Ref<CommandPool>		s_TransferCommandPool;
		static CommandBuffer*		s_TransferCommandBuffer;
		static Ref<CommandPool>		s_GraphicsCommandPool;
		static CommandBuffer*		s_GraphicsCommandBuffer;
		static Ref<Semaphore>		s_Semaphore;
	};
}