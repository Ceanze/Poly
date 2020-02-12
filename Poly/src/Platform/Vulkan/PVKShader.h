#pragma once
#include "polypch.h"

#include <vulkan/vulkan.h>

namespace Poly
{
	class PVKInstance;

	class PVKShader
	{
	public:
		enum class Type { VERTEX, FRAGMENT, COMPUTE };

		PVKShader();
		~PVKShader();

		void init(PVKInstance* instance);
		void cleanup();

		void addStage(Type type, const std::string& shaderName);
		VkPipelineShaderStageCreateInfo getShaderCreateInfo(Type type) const;
		std::vector<VkPipelineShaderStageCreateInfo> getShaderCreateInfos();

	private:
		void createShaderModule(Type type, const std::vector<char>& code);

		std::unordered_map<Type, VkPipelineShaderStageCreateInfo> shaderStages;
		PVKInstance* instance;
	};

}