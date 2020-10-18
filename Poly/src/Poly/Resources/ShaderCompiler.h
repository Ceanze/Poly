#pragma once

#include <string>
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
		std::string m_ShaderFolder = ""; 
	};
}