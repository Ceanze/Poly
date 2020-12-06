#pragma once
#include "polypch.h"
#include "PVKTypes.h"

namespace Poly
{
	class PVKShader
	{
	public:
		PVKShader();
		~PVKShader();

		void Init();
		void Cleanup();

		void AddStage(ShaderStage shaderStage, std::string shaderName);
		VkPipelineShaderStageCreateInfo GetShaderCreateInfo(ShaderStage shaderStage) const;
		std::vector<VkPipelineShaderStageCreateInfo> GetShaderCreateInfos();

	private:
		void CreateShaderModule(ShaderStage shaderStage, const std::vector<char>& code);

		std::unordered_map<ShaderStage, VkPipelineShaderStageCreateInfo> m_ShaderStages;
		std::unordered_map<ShaderStage, std::string> m_ShaderPaths;
	};

}