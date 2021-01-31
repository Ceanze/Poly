#pragma once

#include <vector>
#include <string>

#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class Shader;

	class ResourceLoader
	{
	public:
		ResourceLoader() = default;
		~ResourceLoader() = default;

		static void Init();
		static void Release();

		static Ref<Shader> LoadShader(const std::string& path, FShaderStage shaderStage);

	private:
		static bool s_GLSLInit;
	};
}