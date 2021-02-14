#pragma once

#include <vector>
#include <string>

#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class Shader;
	class Texture;
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

	private:
		static bool s_GLSLInit;

		static Ref<CommandPool>		s_TransferCommandPool;
		static CommandBuffer*		s_TransferCommandBuffer;
		static Ref<CommandPool>		s_GraphicsCommandPool;
		static CommandBuffer*		s_GraphicsCommandBuffer;
		static Ref<Semaphore>		s_Semaphore;
	};
}