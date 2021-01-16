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

		void AddStage(FShaderStage shaderStage, std::string shaderName);
		VkPipelineShaderStageCreateInfo GetShaderCreateInfo(FShaderStage shaderStage) const;
		std::vector<VkPipelineShaderStageCreateInfo> GetShaderCreateInfos();

	private:
		void CreateShaderModule(FShaderStage shaderStage, const std::vector<char>& code);

		std::unordered_map<FShaderStage, VkPipelineShaderStageCreateInfo> m_ShaderStages;
		std::unordered_map<FShaderStage, std::string> m_ShaderPaths;
	};

}