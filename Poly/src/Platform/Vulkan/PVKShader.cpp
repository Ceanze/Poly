#include "polypch.h"
#include "PVKShader.h"
#include "PVKInstance.h"
#include "VulkanCommon.h"

// TODO: Remove, only for testing online spirv compiler
// #include "Poly/Resources/ShaderCompiler.h"
#include "Poly/Resources/ResourceLoader.h"

namespace Poly
{

	PVKShader::PVKShader()
	{
	}

	PVKShader::~PVKShader()
	{
	}

	void PVKShader::Init()
	{
		for (auto& shader : m_ShaderPaths) {
			// std::vector<char> code = readFile("./../assets/shaders/" + shader.second);
			// std::vector<char> code = ShaderCompiler::CompileGLSL(shader.second, "./../assets/shaders/", shader.first);
			std::vector<char> code = ResourceLoader::LoadShader("./../assets/shaders/" + shader.second, shader.first);
			CreateShaderModule(shader.first, code);
		}
		m_ShaderPaths.clear();
	}

	void PVKShader::Cleanup()
	{
		for (auto shaderStage : m_ShaderStages)
			vkDestroyShaderModule(PVKInstance::GetDevice(), shaderStage.second.module, nullptr);
	}

	void PVKShader::AddStage(ShaderStage shaderStage, std::string shaderName)
	{
		// TODO: Have this somewhere else? And fix it to relative path!
		const std::string shaderPath = ".\\..\\assets\\shaders\\";
		m_ShaderPaths[shaderStage] = shaderName;
	}

	VkPipelineShaderStageCreateInfo PVKShader::GetShaderCreateInfo(ShaderStage shaderStage) const
	{
		auto it = m_ShaderStages.find(shaderStage);

		if (it == m_ShaderStages.end())
			POLY_ERROR("Could not find the desired shader stage!");

		return it->second;
	}

	std::vector<VkPipelineShaderStageCreateInfo> PVKShader::GetShaderCreateInfos()
	{
		std::vector<VkPipelineShaderStageCreateInfo> infos;

		for (auto shader : m_ShaderStages)
			infos.push_back(shader.second);

		return infos;
	}

	void PVKShader::CreateShaderModule(ShaderStage shaderStage, const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		PVK_CHECK(vkCreateShaderModule(PVKInstance::GetDevice(), &createInfo, nullptr, &shaderModule), "Failed to create shader module!");

		VkPipelineShaderStageCreateInfo shaderStageInfo = {};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = ConvertShaderStageVK(shaderStage);
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = "main"; // Main should always be considered default

		m_ShaderStages[shaderStage] = shaderStageInfo;
	}

}