#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Poly/Resources/Shader/ShaderReflection.h"

class SpvReflectShaderModule;

namespace Poly
{
	class ShaderReflector
	{
	public:
		ShaderReflector(const std::vector<byte>& data);
		~ShaderReflector();

		ShaderReflection Reflect();

	private:
		bool m_isValid = false;
		Unique<SpvReflectShaderModule> m_Module;
	};
}