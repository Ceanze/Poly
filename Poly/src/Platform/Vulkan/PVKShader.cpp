#include "polypch.h"
#include "PVKShader.h"
#include "PVKInstance.h"
#include "VulkanCommon.h"

namespace Poly
{

	PVKShader::PVKShader() : 
		instance(nullptr)
	{
	}

	PVKShader::~PVKShader()
	{
	}

	void PVKShader::init(PVKInstance* instance)
	{
		this->instance = instance;

		for (auto& shader : this->shaderPaths) {
			std::vector<char> code = readFile(shader.second);
			createShaderModule(shader.first, code);
		}
		this->shaderPaths.clear();
	}

	void PVKShader::cleanup()
	{
		for (auto shaderStage : this->shaderStages)
			vkDestroyShaderModule(this->instance->getDevice(), shaderStage.second.module, nullptr);
	}

	void PVKShader::addStage(Type type, std::string shaderName)
	{
		// TODO: Have this somewhere else? And fix it to relative path!
		const std::string shaderPath = ".\\shaders\\";
		this->shaderPaths[type] = shaderPath + shaderName;
	}

	VkPipelineShaderStageCreateInfo PVKShader::getShaderCreateInfo(Type type) const
	{
		auto& it = this->shaderStages.find(type);

		if (it == this->shaderStages.end())
			POLY_ERROR("Could not find the desired shader type!");

		return it->second;
	}

	std::vector<VkPipelineShaderStageCreateInfo> PVKShader::getShaderCreateInfos()
	{
		std::vector<VkPipelineShaderStageCreateInfo> infos;

		for (auto shader : this->shaderStages)
			infos.push_back(shader.second);

		return infos;
	}

	void PVKShader::createShaderModule(Type type, const std::vector<char>& code)
	{
		VkShaderStageFlagBits shaderStage;
		switch (type) {
		case Type::VERTEX:
			shaderStage = VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case Type::FRAGMENT:
			shaderStage = VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		case Type::COMPUTE:
			shaderStage = VK_SHADER_STAGE_COMPUTE_BIT;
			break;
		}

		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		PVK_CHECK(vkCreateShaderModule(this->instance->getDevice(), &createInfo, nullptr, &shaderModule), "Failed to create shader module!");

		VkPipelineShaderStageCreateInfo shaderStageInfo = {};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = shaderStage;
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = "main"; // Main should always be considered default

		this->shaderStages[type] = shaderStageInfo;
	}

}