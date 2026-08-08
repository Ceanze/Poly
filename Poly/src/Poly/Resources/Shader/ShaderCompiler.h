#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"

#include <string>

namespace Poly
{
	class ShaderCompiler
	{
	public:
		ShaderCompiler() = default;

		// Compiles the spirv - should probably return a shader or similar later
		static const std::vector<byte> CompileGLSL(std::string_view path, FShaderStage shaderStage);
	};
} // namespace Poly