#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Poly/Resources/Shader/ShaderReflection.h"

namespace Poly
{
	class Pass;

	class ShaderReflector
	{
	public:
		ShaderReflector();
		~ShaderReflector() = default;

		ShaderReflector& AddReflect(FShaderStage shaderStage, const std::string& path);
		ShaderReflector& AddReflect(FShaderStage shaderStage, const std::vector<char>& data);

		ShaderReflection GenerateReflection();

	private:
		struct ReflectionStage
		{
			FShaderStage ShaderStage;
			std::string ShaderPath;
			std::vector<char> ShaderData;
		};

		std::vector<ReflectionStage> m_ReflectionStages;
	};
}