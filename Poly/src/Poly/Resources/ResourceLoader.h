#pragma once

#include <vector>
#include <string>

#include "Poly/Rendering/Core/API/GraphicsTypes.h"

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
	class Material;
	class Semaphore;
	class CommandPool;
	class CommandBuffer;

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

		static Ref<Model> LoadModel(const std::string& path);

		static Ref<Material> LoadMaterial(const std::string& path);

	private:
		static void ProcessNode(aiNode* pNode, const aiScene* pScene, Model* pModel);
		static void ProcessMesh(aiMesh* pMesh, const aiScene* pScene, Mesh* pPolyMesh);
		static void ProcessMaterial(aiMaterial* pMaterial, const aiScene* pScene, PolyID& materialID);
		static void TransferDataToGPU(const void* data, uint32 size, uint32 count, Ref<Buffer> pDestinationBuffer);
		static glm::mat4 ConvertAiMatToGLM(const void* pMat);

		static bool s_GLSLInit;

		static Ref<CommandPool>		s_TransferCommandPool;
		static CommandBuffer*		s_TransferCommandBuffer;
		static Ref<CommandPool>		s_GraphicsCommandPool;
		static CommandBuffer*		s_GraphicsCommandBuffer;
		static Ref<Semaphore>		s_Semaphore;
	};
}