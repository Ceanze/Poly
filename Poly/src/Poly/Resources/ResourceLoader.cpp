#include "polypch.h"
#include "ResourceLoader.h"
#include "Platform/API/Shader.h"
#include "Platform/API/Buffer.h"
#include "Platform/API/Texture.h"
#include "Platform/API/Semaphore.h"
#include "Platform/API/CommandPool.h"
#include "Platform/API/CommandQueue.h"
#include "Platform/API/CommandBuffer.h"
#include "Poly/Core/RenderAPI.h"

#include "GLSLang.h"

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
		RenderAPI::GetCommandQueue(FQueueType::TRANSFER)->Wait();
		RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Wait();

		stbi_image_free(data);

		return texture;
	}

}