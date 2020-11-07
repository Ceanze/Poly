#include "polypch.h"
#include "ShaderCompiler.h"

#include <fstream>

namespace Poly
{
	ShaderCompiler::ShaderCompiler()
	{
		glslang::TShader shader(EShLangVertex);
	}
	std::string ShaderCompiler::GetFileFolder(const std::string& path)
	{
		const size_t lastPos = path.find_last_of("/\\");
		return path.substr(0, lastPos);
	}
}