#include "polypch.h"
#include "ShaderCompiler.h"
#include <glslang/StandAlone/DirStackFileIncluder.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/MachineIndependent/reflection.h>

namespace Poly
{
	ShaderCompiler::ShaderCompiler()
	{
		glslang::TShader shader(EShLangVertex);
	}
}