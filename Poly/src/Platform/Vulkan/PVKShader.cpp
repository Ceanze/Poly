#include "polypch.h"
#include "PVKShader.h"
#include "PVKInstance.h"
#include "VulkanCommon.h"

#include "Poly/Resources/ResourceLoader.h"

namespace Poly
{

	PVKShader::~PVKShader()
	{
		vkDestroyShaderModule(PVKInstance::GetDevice(), m_ShaderModule, nullptr);
	}

	void PVKShader::Init(const ShaderDesc* pDesc)
	{
		m_ShaderStage = pDesc->ShaderStage;

		// Create shader module
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType	= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize	= pDesc->ShaderCode.size();
		createInfo.pCode	= reinterpret_cast<const uint32_t*>(pDesc->ShaderCode.data());
		createInfo.flags	= 0;
		createInfo.pNext	= nullptr;

		PVK_CHECK(vkCreateShaderModule(PVKInstance::GetDevice(), &createInfo, nullptr, &m_ShaderModule), "Failed to create shader module!");

		// Save the pipeline info for easier use later on
		m_PipelineInfo.sType	= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		m_PipelineInfo.stage	= ConvertShaderStageVK(pDesc->ShaderStage);
		m_PipelineInfo.module	= m_ShaderModule;
		m_PipelineInfo.pName	= pDesc->EntryPoint.c_str();
	}
}