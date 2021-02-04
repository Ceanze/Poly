#pragma once
#include "polypch.h"
#include "PVKTypes.h"

#include "Platform/API/Shader.h"

namespace Poly
{
	class PVKShader : public Shader
	{
	public:
		PVKShader() = default;
		~PVKShader();

		virtual void Init(const ShaderDesc* pDesc) override final;

		FShaderStage GetShaderStage() const { return m_ShaderStage; }
		VkPipelineShaderStageCreateInfo GetPipelineInfo() const { return m_PipelineInfo; }
		VkShaderModule GetNativeVK() const { return m_ShaderModule; }
		virtual uint64 GetNative() const override final { return reinterpret_cast<uint64>(m_ShaderModule); }

	private:
		VkShaderModule m_ShaderModule = VK_NULL_HANDLE;
		VkPipelineShaderStageCreateInfo m_PipelineInfo = {};
		FShaderStage m_ShaderStage = FShaderStage::NONE;
		std::string m_EntryPoint = "";
	};

}