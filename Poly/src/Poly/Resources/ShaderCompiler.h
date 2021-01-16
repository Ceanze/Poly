#pragma once

#include <string>

#include "Poly/Resources/GLSLang.h"

/*
	This file is in TESTING PHASE
*/

namespace Poly
{
	class ShaderCompiler
	{
	public:
		ShaderCompiler() = default;
		~ShaderCompiler();

		// Compiles the spirv - should probably return a shader or similar later
		static const std::vector<char> CompileGLSL(const std::string& filename, const std::string& folder, FShaderStage shaderStage);

	private:
		static bool s_glslInit;
	};
}