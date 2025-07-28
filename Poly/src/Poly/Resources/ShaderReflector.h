#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Poly/Rendering/RenderGraph/PassReflection.h"

namespace Poly
{
	class Pass;

	class ShaderReflector
	{
	public:
		ShaderReflector();
		~ShaderReflector() = default;

		void Reflect(FShaderStage shaderStage, const std::string& path);

		PassReflection GenerateReflection();
	};
}