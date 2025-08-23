#include "ShaderManager.h"

#include "Poly/Resources/ResourceLoader.h"

namespace Poly
{
	std::map<PolyID, ShaderManager::ShaderData> ShaderManager::s_Shaders = {};

	void ShaderManager::Init()
	{

	}

	void ShaderManager::Release()
	{
		s_Shaders.clear();
	}

	PolyID ShaderManager::CreateShader(std::string_view path, FShaderStage shaderStage)
	{
		PolyID hash{ std::hash<std::string_view>{}(path) };

		if (s_Shaders.contains(hash))
			return hash;

		Ref<Shader> shader = ResourceLoader::LoadShader(path, shaderStage);
		s_Shaders[hash] = { shader };

		return hash;
	}

	Ref<Shader> ShaderManager::GetShader(PolyID shaderID)
	{
		if (const auto& shader = s_Shaders.find(shaderID); shader != s_Shaders.end())
			return shader->second.pShader;

		POLY_CORE_ERROR("Cannot get shader with shaderID {}, it does not exist", shaderID);
		return nullptr;
	}
}