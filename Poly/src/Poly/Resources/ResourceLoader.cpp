#include "polypch.h"
#include "ResourceLoader.h"
#include "ResourceManager.h"
#include "Platform/API/Shader.h"
#include "Platform/API/Buffer.h"
#include "Platform/API/Texture.h"
#include "Platform/API/Semaphore.h"
#include "Platform/API/TextureView.h"
#include "Platform/API/CommandPool.h"
#include "Platform/API/CommandQueue.h"
#include "Platform/API/CommandBuffer.h"
#include "Poly/Core/RenderAPI.h"

#include "Poly/Model/Mesh.h"
#include "Poly/Model/Model.h"
#include "Poly/Model/Material.h"

#include "GLSLang.h"
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/material.h>
#include <assimp/Importer.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Poly
{
	bool				ResourceLoader::s_GLSLInit				= false;
	Ref<CommandPool>	ResourceLoader::s_TransferCommandPool	= nullptr;
	CommandBuffer*		ResourceLoader::s_TransferCommandBuffer	= nullptr;
	Ref<CommandPool>	ResourceLoader::s_GraphicsCommandPool	= nullptr;
	CommandBuffer*		ResourceLoader::s_GraphicsCommandBuffer	= nullptr;
	Ref<Semaphore>		ResourceLoader::s_Semaphore				= nullptr;

	constexpr const char* SHADER_PATH = "../assets/shaders/";
	constexpr const char* ASSETS_PATH = "../assets/";

	void ResourceLoader::Init()
	{
		// GLSL
		s_GLSLInit = glslang::InitializeProcess();
		if (!s_GLSLInit)
			POLY_CORE_ERROR("[ResourceLoader]: Failed to initialize glslang! No shaders will be loaded!");

		// Command pools and buffers
		s_TransferCommandPool	= RenderAPI::CreateCommandPool(FQueueType::TRANSFER, FCommandPoolFlags::NONE);
		s_TransferCommandBuffer	= s_TransferCommandPool->AllocateCommandBuffer(ECommandBufferLevel::PRIMARY);

		s_GraphicsCommandPool	= RenderAPI::CreateCommandPool(FQueueType::GRAPHICS, FCommandPoolFlags::NONE);
		s_GraphicsCommandBuffer	= s_GraphicsCommandPool->AllocateCommandBuffer(ECommandBufferLevel::PRIMARY);

		// Semaphore
		s_Semaphore				= RenderAPI::CreateSemaphore();
	}

	void ResourceLoader::Release()
	{
		if (s_GLSLInit)
			glslang::FinalizeProcess();

		// Wait for device to be done before calling the destructor for commandpool and buffer
		RenderAPI::GetCommandQueue(FQueueType::TRANSFER)->Wait();
		RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Wait();

		// Since ResourceLoader is a static class the destructor will be called after the release
		// of the graphics device. Therefore the graphics resources are manually deleted
		s_TransferCommandPool.reset();
		s_GraphicsCommandPool.reset();
		s_Semaphore.reset();
	}


	Ref<Shader> ResourceLoader::LoadShader(const std::string& path, FShaderStage shaderStage)
	{
		if (!s_GLSLInit)
		{
			POLY_CORE_ERROR("[ResourceLoader]: Failed to load shader, GLSL is not correctly initilized!");
			return nullptr;
		}

		EShLanguage shaderType = ConvertShaderStageGLSLang(shaderStage);

		std::string realPath = SHADER_PATH + path;

		// Extract folder and filename
		size_t slashPos = realPath.find_last_of("/\\");
		std::string folder = realPath.substr(0, slashPos);
		std::string filename = realPath.substr(slashPos + 1);

		// Load and transfer content to string
		std::ifstream file(realPath);

		POLY_VALIDATE(file.is_open(), "Failed to open shader file: {0} \n at path: {1}", filename, folder);

		const std::string inputGLSL((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		const char* pInputCString = inputGLSL.c_str();

		// Setup glslang shader
		glslang::TShader shader(shaderType);

		shader.setStrings(&pInputCString, 1);

		// Setup resources (might save values elsewhere or as constants)
		int clientInputSemanticsVersion = 100;
		glslang::EShTargetClientVersion vulkanClientVersion	= glslang::EShTargetVulkan_1_2; // VULKAN 1.2 (latest)
		glslang::EShTargetLanguageVersion targetVersion		= glslang::EShTargetSpv_1_5; // SPV 1.5 (latest)
		const TBuiltInResource* pResources					= GetDefaultBuiltInResources();
		EShMessages messages								= static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules | EShMsgDefault);
		const int defaultGLSLVersion						= 450; // Shader version 450 (latest)

		shader.setEnvInput(glslang::EShSourceGlsl, shaderType, glslang::EShClientVulkan, clientInputSemanticsVersion);
		shader.setEnvClient(glslang::EShClientVulkan, vulkanClientVersion);
		shader.setEnvTarget(glslang::EShTargetSpv, targetVersion);

		DirStackFileIncluder includer;
		includer.pushExternalLocalDirectory(folder);

		std::string preprocessedGLSL;

		if (!shader.preprocess(pResources, defaultGLSLVersion, ENoProfile, false, false, messages, &preprocessedGLSL, includer))
			POLY_CORE_WARN("GLSL preprocessing failed for: {0} \n {1} \n {2}", filename, shader.getInfoLog(), shader.getInfoDebugLog());

		const char* pPreprocessedCString = preprocessedGLSL.c_str();
		shader.setStrings(&pPreprocessedCString, 1);

		if (!shader.parse(pResources, defaultGLSLVersion, false, messages))
			POLY_CORE_WARN("GLSL parsing failed for: {0} \n {1} \n {2}", filename, shader.getInfoLog(), shader.getInfoDebugLog());

		glslang::TProgram program;
		program.addShader(&shader);

		if (!program.link(messages))
			POLY_CORE_WARN("GLSL linking failed for: {0} \n {1} \n {2}", filename, shader.getInfoLog(), shader.getInfoDebugLog());

		std::vector<uint32_t> sprirv;
		spv::SpvBuildLogger logger;
		glslang::SpvOptions spvOptions;
		glslang::GlslangToSpv(*program.getIntermediate(shaderType), sprirv, &logger, &spvOptions);

		const uint32_t sourceSize = static_cast<uint32_t>(sprirv.size()) * sizeof(uint32_t);
		std::vector<char> correctType = std::vector<char>(reinterpret_cast<char*>(sprirv.data()), reinterpret_cast<char*>(sprirv.data()) + sourceSize);

		ShaderDesc desc = {};
		desc.EntryPoint		= "main";
		desc.ShaderCode		= correctType;
		desc.ShaderStage	= shaderStage;
		Ref<Shader> polyShader = RenderAPI::CreateShader(&desc);

		return polyShader;
	}


	std::vector<byte> ResourceLoader::LoadRawImage(const std::string& path)
	{
		std::string realPath = ASSETS_PATH + path;

		int texWidth	= 0;
		int texHeight	= 0;
		int channels	= 0;

		byte* data = stbi_load(path.c_str(), &texWidth, &texHeight, &channels, 0);
		if (!data)
			POLY_CORE_ERROR("Failed to load image {}", path);

		std::vector<byte> image(texWidth * texHeight * channels);
		memcpy(image.data(), data, texWidth * texHeight * channels);

		return image;
	}

	Ref<Texture> ResourceLoader::LoadTexture(const std::string& path, EFormat format)
	{
		std::string realPath = ASSETS_PATH + path;

		// Load image
		int texWidth	= 0;
		int texHeight	= 0;
		int channels	= 0;

		byte* data = stbi_load(realPath.c_str(), &texWidth, &texHeight, &channels, 0);
		if (!data)
		{
			POLY_VALIDATE(false, "Failed to load image {}", path);
		}

		// Create texture
		TextureDesc textureDesc = {};
		textureDesc.Width			= texWidth;
		textureDesc.Height			= texHeight;
		textureDesc.Depth			= 1;
		textureDesc.MemoryUsage		= EMemoryUsage::GPU_ONLY;
		textureDesc.SampleCount		= 1;
		textureDesc.ArrayLayers		= 1;
		textureDesc.MipLevels		= 1;
		textureDesc.TextureDim		= ETextureDim::DIM_2D;
		textureDesc.TextureUsage	= FTextureUsage::TRANSFER_DST | FTextureUsage::TRANSFER_SRC | FTextureUsage::SAMPLED;
		textureDesc.Format			= format;
		Ref<Texture> texture = RenderAPI::CreateTexture(&textureDesc);

		// Create transfer buffer
		BufferDesc bufferDesc = {};
		bufferDesc.BufferUsage	= FBufferUsage::TRANSFER_SRC;
		bufferDesc.MemUsage		= EMemoryUsage::CPU_VISIBLE;
		bufferDesc.Size			= texWidth * texHeight * channels;
		Ref<Buffer> buffer = RenderAPI::CreateBuffer(&bufferDesc);

		// Map transfer buffer
		void* buffMap = buffer->Map();
		memcpy(buffMap, data, texWidth * texHeight * channels);
		buffer->Unmap();

		// Copy over data from buffer to texture
		s_TransferCommandPool->Reset();
		s_TransferCommandBuffer->Begin(FCommandBufferFlag::ONE_TIME_SUBMIT);

		// 1. A newly created texture is created with the layout "UNDEFINED". This needs to be transfered to TRANSFER_DST before transfer
		s_TransferCommandBuffer->PipelineTextureBarrier(
			texture.get(),
			FPipelineStage::TOP_OF_PIPE,
			FPipelineStage::TRANSFER,
			FAccessFlag::NONE,
			FAccessFlag::TRANSFER_WRITE,
			ETextureLayout::UNDEFINED,
			ETextureLayout::TRANSFER_DST_OPTIMAL
		);

		// 2. When in correct layout, record transfer command
		CopyBufferDesc copyDesc = {};
		copyDesc.Width		= texWidth;
		copyDesc.Height		= texHeight;
		copyDesc.Depth		= 1;
		copyDesc.ArrayCount	= 1;
		copyDesc.ArrayLayer	= 0;
		copyDesc.MipLevel	= 0;
		s_TransferCommandBuffer->CopyBufferToTexture(buffer.get(), texture.get(), ETextureLayout::TRANSFER_DST_OPTIMAL, copyDesc);

		// 3. Release texture from transfer queue
		s_TransferCommandBuffer->ReleaseTexture(
			texture.get(),
			FPipelineStage::TRANSFER,
			FPipelineStage::TRANSFER,
			FAccessFlag::TRANSFER_READ,
			ETextureLayout::TRANSFER_DST_OPTIMAL,
			ETextureLayout::SHADER_READ_ONLY_OPTIMAL,
			RenderAPI::GetCommandQueue(FQueueType::TRANSFER)->GetQueueFamilyIndex(),
			RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->GetQueueFamilyIndex()
		);
		s_TransferCommandBuffer->End();

		// 4. Semaphore to make sure the transfer is done before acquire
		s_Semaphore->ClearWaitStageMask();
		s_Semaphore->AddWaitStageMask(FPipelineStage::FRAGMENT_SHADER);

		// 5. Submit to transfer queue
		RenderAPI::GetCommandQueue(FQueueType::TRANSFER)->Submit(s_TransferCommandBuffer, nullptr, s_Semaphore.get(), nullptr);

		// 6. Acquire texture to graphics queue
		s_GraphicsCommandPool->Reset();
		s_GraphicsCommandBuffer->Begin(FCommandBufferFlag::ONE_TIME_SUBMIT);

		s_GraphicsCommandBuffer->AcquireTexture(
			texture.get(),
			FPipelineStage::TRANSFER,
			FPipelineStage::TRANSFER,
			FAccessFlag::TRANSFER_READ,
			ETextureLayout::TRANSFER_DST_OPTIMAL,
			ETextureLayout::SHADER_READ_ONLY_OPTIMAL,
			RenderAPI::GetCommandQueue(FQueueType::TRANSFER)->GetQueueFamilyIndex(),
			RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->GetQueueFamilyIndex()
		);
		s_GraphicsCommandBuffer->End();

		// 7. Submit to graphics queue
		RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Submit(s_GraphicsCommandBuffer, s_Semaphore.get(), nullptr, nullptr);

		// 8. Wait on both queues (or something better, this is the simple approach)
		// TODO: Use semaphore here instead!
		RenderAPI::GetCommandQueue(FQueueType::TRANSFER)->Wait();
		RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Wait();

		stbi_image_free(data);

		return texture;
	}

	Ref<Model> ResourceLoader::LoadModel(const std::string& path)
	{
		Assimp::Importer importer;
		const aiScene* pScene = importer.ReadFile(path, 0);

		if (!pScene)
		{
			POLY_CORE_WARN("Could not open mesh at path {}", path);
			return nullptr;
		}

		Ref<Model> pModel = CreateRef<Model>();

		ProcessNode(pScene->mRootNode, pScene, pModel.get());

		return pModel;
	}

	Ref<Material> ResourceLoader::LoadMaterial(const std::string& path)
	{
		return nullptr;
	}

	void ResourceLoader::ProcessNode(aiNode *pNode, const aiScene *pScene, Model* pModel)
	{
		for (uint32 i = 0; i < pNode->mNumMeshes; i++)
		{
			aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];
			aiMaterial* pMaterial = pScene->mMaterials[pMesh->mMaterialIndex];
			Ref<Mesh> pPolyMesh = Mesh::Create();
			PolyID materialID = 0;
			ProcessMesh(pMesh, pScene, pPolyMesh.get());
			ProcessMaterial(pMaterial, pScene, materialID);
			pModel->AddMeshInstance({ pPolyMesh, materialID });
		}

		for (uint32 i = 0; i < pNode->mNumChildren; i++)
		{
			aiNode* child = pNode->mChildren[i];
			ProcessNode(child, pScene, pModel);
		}
	}

	void ResourceLoader::ProcessMesh(aiMesh* pMesh, const aiScene* pScene, Mesh* pPolyMesh)
	{
		std::vector<Vertex> vertices(pMesh->mNumVertices);
		std::vector<uint32> indices(pMesh->mNumFaces);

		for (uint32 i = 0; i < pMesh->mNumVertices; i++)
		{
			vertices[i].Position.x = pMesh->mVertices[i].x;
			vertices[i].Position.y = pMesh->mVertices[i].y;
			vertices[i].Position.z = pMesh->mVertices[i].z;

			if (pMesh->HasNormals())
			{
				vertices[i].Normal.x = pMesh->mNormals[i].x;
				vertices[i].Normal.y = pMesh->mNormals[i].y;
				vertices[i].Normal.z = pMesh->mNormals[i].z;
			}

			// There can be different sets of texture coords - unsure of purpose - only use the first
			if (pMesh->HasTextureCoords(0))
			{
				vertices[i].TexCoord.x = pMesh->mTextureCoords[0][i].x;
				vertices[i].TexCoord.y = pMesh->mTextureCoords[0][i].y;
			}

			if (pMesh->HasTangentsAndBitangents())
			{
				vertices[i].Tangent.x = pMesh->mTangents[i].x;
				vertices[i].Tangent.y = pMesh->mTangents[i].y;
				vertices[i].Tangent.z = pMesh->mTangents[i].z;
			}
		}

		for (uint32 i = 0; i < pMesh->mNumFaces; i++)
		{
			indices[i * 3 + 0] = pMesh->mFaces[i].mIndices[0];
			indices[i * 3 + 1] = pMesh->mFaces[i].mIndices[1];
			indices[i * 3 + 2] = pMesh->mFaces[i].mIndices[2];
		}

		// Create and transfer data to buffers
		// Vertices
		BufferDesc desc = {};
		desc.BufferUsage	= FBufferUsage::TRANSFER_DST | FBufferUsage::VERTEX_BUFFER;
		desc.MemUsage		= EMemoryUsage::GPU_ONLY;
		desc.Size			= sizeof(Vertex) * vertices.size();
		Ref<Buffer> pVertexBuffer = RenderAPI::CreateBuffer(&desc);
		TransferDataToGPU(vertices.data(), vertices.size(), sizeof(Vertex), pVertexBuffer);

		// Indices
		desc.BufferUsage	= FBufferUsage::TRANSFER_DST | FBufferUsage::INDEX_BUFFER;
		desc.MemUsage		= EMemoryUsage::GPU_ONLY;
		desc.Size			= sizeof(uint32) * indices.size();
		Ref<Buffer> pIndexBuffer = RenderAPI::CreateBuffer(&desc);
		TransferDataToGPU(indices.data(), indices.size(), sizeof(uint32), pIndexBuffer);

		pPolyMesh->SetVertexBuffer(pVertexBuffer);
		pPolyMesh->SetIndexBuffer(pIndexBuffer);
	}

	void ResourceLoader::ProcessMaterial(aiMaterial* pMaterial, const aiScene* pScene, PolyID& materialID)
	{
		if (ResourceManager::IsResourceLoaded(pMaterial->GetName().C_Str()))
		{
			// Load material returns a loaded value if the material is already loaded
			materialID = ResourceManager::LoadMaterial(pMaterial->GetName().C_Str());
			return;
		}

		Ref<Material> pPolyMaterial = CreateRef<Material>();
		bool hasTextures = false;

		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString path;
			pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path);
			PolyID id = ResourceManager::LoadTexture(path.C_Str(), EFormat::R8G8B8A8_UNORM);

			TextureViewDesc desc = {};
			desc.pTexture			= ResourceManager::GetTexture(id);
			desc.ArrayLayer			= 0;
			desc.ArrayLayerCount	= 1;
			desc.MipLevel			= 0;
			desc.MipLevelCount		= 1;
			desc.ImageViewFlag		= FImageViewFlag::COLOR;
			desc.ImageViewType		= EImageViewType::TYPE_2D;
			desc.Format				= EFormat::R8G8B8A8_UNORM;

			Ref<TextureView> pTextureView = RenderAPI::CreateTextureView(&desc);

			pPolyMaterial->SetTexture(Material::Type::ALBEDO, desc.pTexture);
			pPolyMaterial->SetTextureView(Material::Type::ALBEDO, pTextureView);
			hasTextures = true;
		}

		if (!hasTextures)
		{
			POLY_CORE_WARN("No valid textures were found for material {}!", pMaterial->GetName().C_Str());
			return;
		}

		materialID = ResourceManager::RegisterMaterial(pMaterial->GetName().C_Str(), pPolyMaterial);
	}

	void ResourceLoader::TransferDataToGPU(const void* data, uint32 size, uint32 count, Ref<Buffer> pDestinationBuffer)
	{
		// Create transfer buffer
		BufferDesc bufferDesc = {};
		bufferDesc.BufferUsage	= FBufferUsage::TRANSFER_SRC;
		bufferDesc.MemUsage		= EMemoryUsage::CPU_VISIBLE;
		bufferDesc.Size			= size;
		Ref<Buffer> pBuffer = RenderAPI::CreateBuffer(&bufferDesc);

		// Map transfer buffer
		void* buffMap = pBuffer->Map();
		memcpy(buffMap, data, size * count);
		pBuffer->Unmap();

		// Copy over data from buffer to texture
		s_TransferCommandPool->Reset();
		s_TransferCommandBuffer->Begin(FCommandBufferFlag::ONE_TIME_SUBMIT);
		s_TransferCommandBuffer->CopyBuffer(pBuffer.get(), pDestinationBuffer.get(), size, 0, 0);
		s_TransferCommandBuffer->ReleaseBuffer(
			pDestinationBuffer.get(),
			FPipelineStage::TRANSFER,
			FPipelineStage::TRANSFER,
			FAccessFlag::TRANSFER_READ,
			RenderAPI::GetCommandQueue(FQueueType::TRANSFER)->GetQueueFamilyIndex(),
			RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->GetQueueFamilyIndex()
		);
		s_TransferCommandBuffer->End();

		// Semaphore to make sure the transfer is done before acquire
		s_Semaphore->ClearWaitStageMask();
		s_Semaphore->AddWaitStageMask(FPipelineStage::VERTEX_SHADER);

		// Submit to transfer queue
		RenderAPI::GetCommandQueue(FQueueType::TRANSFER)->Submit(s_TransferCommandBuffer, nullptr, s_Semaphore.get(), nullptr);

		// Acquire texture to graphics queue
		s_GraphicsCommandPool->Reset();
		s_GraphicsCommandBuffer->Begin(FCommandBufferFlag::ONE_TIME_SUBMIT);
		s_TransferCommandBuffer->AcquireBuffer(
			pDestinationBuffer.get(),
			FPipelineStage::TRANSFER,
			FPipelineStage::TRANSFER,
			FAccessFlag::TRANSFER_READ,
			RenderAPI::GetCommandQueue(FQueueType::TRANSFER)->GetQueueFamilyIndex(),
			RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->GetQueueFamilyIndex()
		);
		s_GraphicsCommandBuffer->End();

		// Submit to graphics queue
		RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Submit(s_GraphicsCommandBuffer, s_Semaphore.get(), nullptr, nullptr);

		// Wait on both queues (or something better, this is the simple approach)
		// TODO: Use semaphore here instead!
		RenderAPI::GetCommandQueue(FQueueType::TRANSFER)->Wait();
		RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Wait();
	}
}