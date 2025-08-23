#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Poly/Resources/Shader/ShaderReflection.h"

namespace Poly
{
	class Pass;

	class ShaderReflector
	{
	public:
		ShaderReflector() = default;
		~ShaderReflector() = default;

		static ShaderReflection Reflect(FShaderStage shaderStage, const std::vector<byte>& data);
	};
}