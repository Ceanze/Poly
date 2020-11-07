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
		ShaderCompiler();
		~ShaderCompiler() = default;

	private:
		std::string GetFileFolder(const std::string& path);

	private:
		std::string m_ShaderFolder = ""; 
	};
}