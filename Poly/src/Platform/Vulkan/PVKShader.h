#pragma once
#include "polypch.h"
#include "PVKTypes.h"

#include <vulkan/vulkan.h>

namespace Poly
{
	class PVKShader
	{
	public:
		//enum class Type { VERTEX, FRAGMENT, COMPUTE };

		PVKShader();
		~PVKShader();

		void init();
		void cleanup();

		void addStage(ShaderStage shaderStage, std::string shaderName);
		VkPipelineShaderStageCreateInfo getShaderCreateInfo(ShaderStage shaderStage) const;
		std::vector<VkPipelineShaderStageCreateInfo> getShaderCreateInfos();

	private:
		void createShaderModule(ShaderStage shaderStage, const std::vector<char>& code);

		std::unordered_map<ShaderStage, VkPipelineShaderStageCreateInfo> shaderStages;
		std::unordered_map<ShaderStage, std::string> shaderPaths;
	};

}