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

		void addStage(ShaderType type, std::string shaderName);
		VkPipelineShaderStageCreateInfo getShaderCreateInfo(ShaderType type) const;
		std::vector<VkPipelineShaderStageCreateInfo> getShaderCreateInfos();

	private:
		void createShaderModule(ShaderType type, const std::vector<char>& code);

		std::unordered_map<ShaderType, VkPipelineShaderStageCreateInfo> shaderStages;
		std::unordered_map<ShaderType, std::string> shaderPaths;
	};

}