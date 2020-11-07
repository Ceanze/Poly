#include "polypch.h"
#include "PVKShader.h"
#include "PVKInstance.h"
#include "VulkanCommon.h"

namespace Poly
{

	PVKShader::PVKShader()
	{
	}

	PVKShader::~PVKShader()
	{
	}

	void PVKShader::init()
	{
		for (auto& shader : this->shaderPaths) {
			std::vector<char> code = readFile(shader.second);
			createShaderModule(shader.first, code);
		}
		this->shaderPaths.clear();
	}

	void PVKShader::cleanup()
	{
		for (auto shaderStage : this->shaderStages)
			vkDestroyShaderModule(PVKInstance::getDevice(), shaderStage.second.module, nullptr);
	}

	void PVKShader::addStage(ShaderStage shaderStage, std::string shaderName)
	{
		// TODO: Have this somewhere else? And fix it to relative path!
		const std::string shaderPath = ".\\..\\assets\\shaders\\";
		this->shaderPaths[shaderStage] = shaderPath + shaderName;
	}

	VkPipelineShaderStageCreateInfo PVKShader::getShaderCreateInfo(ShaderStage shaderStage) const
	{
		auto& it = this->shaderStages.find(shaderStage);

		if (it == this->shaderStages.end())
			POLY_ERROR("Could not find the desired shader stage!");

		return it->second;
	}

	std::vector<VkPipelineShaderStageCreateInfo> PVKShader::getShaderCreateInfos()
	{
		std::vector<VkPipelineShaderStageCreateInfo> infos;

		for (auto shader : this->shaderStages)
			infos.push_back(shader.second);

		return infos;
	}

	void PVKShader::createShaderModule(ShaderStage shaderStage, const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		PVK_CHECK(vkCreateShaderModule(PVKInstance::getDevice(), &createInfo, nullptr, &shaderModule), "Failed to create shader module!");

		VkPipelineShaderStageCreateInfo shaderStageInfo = {};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = ConvertShaderStageVK(shaderStage);
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = "main"; // Main should always be considered default

		this->shaderStages[shaderStage] = shaderStageInfo;
	}

}