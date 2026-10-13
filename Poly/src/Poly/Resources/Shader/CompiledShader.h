#pragma once

#include "Poly/Resources/Shader/ShaderReflection.h"

namespace Poly
{
	struct CompiledShader
	{
		std::vector<byte> Code;
		ShaderReflection  Reflection;
	};
} // namespace Poly
