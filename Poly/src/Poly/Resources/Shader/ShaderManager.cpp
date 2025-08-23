#include "ShaderManager.h"

#include "Platform/API/Shader.h"
#include "Poly/Core/RenderAPI.h"
#include "Poly/Resources/ResourceLoader.h"
#include "Poly/Resources/Shader/ShaderReflector.h"

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

		const std::vector<byte> shaderCode = ResourceLoader::LoadShader(path, shaderStage);

		ShaderDesc desc = {};
		desc.EntryPoint = "main"; // TODO: Make customizable
		desc.ShaderCode = shaderCode;
		desc.ShaderStage = shaderStage;
		Ref<Shader> shader = RenderAPI::CreateShader(&desc);

		ShaderReflector reflector(shaderCode);
		ShaderReflection reflection = reflector.Reflect();

		//{
		//	POLY_INFO("Reflection for shader at path {}", path);
		//	POLY_INFO("Inputs:");
		//	for (const auto& input : reflection.Inputs)
		//		POLY_INFO("\tName: {}, Location: {}", input.Name, input.Location);

		//	POLY_INFO("Bindings:");
		//	for (const auto& binding : reflection.Bindings)
		//		POLY_INFO("\tName: {}, Set: {}, Binding: {}, DescType {}, Count: {}", binding.Name, binding.Set, binding.Binding, static_cast<uint32>(binding.DescriptorType), binding.Count);

		//	POLY_INFO("Push constants:");
		//	for (const auto& ps : reflection.PushConstants)
		//		POLY_INFO("\tName: {}, Size: {}, Offset: {}", ps.Name, ps.Size, ps.Offset);
		//	POLY_TRACE("-------------------------");
		//}

		s_Shaders[hash] = { shader, reflection };

		return hash;
	}

	Ref<Shader> ShaderManager::GetShader(PolyID shaderID)
	{
		if (const auto& shader = s_Shaders.find(shaderID); shader != s_Shaders.end())
			return shader->second.pShader;

		POLY_CORE_ERROR("Shader cannot be gotten, shaderID {} is invalid", shaderID);
		return nullptr;
	}

	const ShaderReflection& ShaderManager::GetReflection(PolyID shaderID)
	{
		const auto& shaderData = s_Shaders.find(shaderID);
		const bool exist = shaderData != s_Shaders.end();
		POLY_VALIDATE(exist, "Shader reflection cannot be gotten, shaderID {} is invalid", shaderID);

		if (exist)
			return shaderData->second.reflection;
	}
}