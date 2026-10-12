#include "PipelineCache.h"

#include "Platform/API/GraphicsPipeline.h"
#include "Platform/API/PipelineLayout.h"
#include "Poly/Core/RenderAPI.h"
#include "Poly/Resources/Shader/ShaderManager.h"
#include "ResourceManager.h"

namespace Poly
{
	PipelineCache::Entry PipelineCache::GetOrCreate(const ResolvedPass& pass, const std::vector<EFormat>& colorFormats, EFormat depthFormat, EFormat stencilFormat)
	{
		std::string key = pass.Name;
		for (EFormat format : colorFormats)
			key += "|c" + std::to_string(static_cast<uint32>(format));
		key += "|d" + std::to_string(static_cast<uint32>(depthFormat));
		key += "|s" + std::to_string(static_cast<uint32>(stencilFormat));

		PipelineLayout* pLayout = GetOrCreateLayout(pass);

		auto it = m_Pipelines.find(key);
		if (it != m_Pipelines.end())
			return {pLayout, it->second.get()};

		GraphicsPipelineDesc desc    = pass.PipelineDesc;
		desc.pPipelineLayout         = pLayout;
		desc.pRenderPass             = nullptr; // dynamic rendering - no VkRenderPass/Framebuffer
		desc.ColorAttachmentFormats  = colorFormats;
		desc.DepthAttachmentFormat   = depthFormat;
		desc.StencilAttachmentFormat = stencilFormat;

		for (const auto& [shaderPath, shaderStage] : pass.Shaders)
		{
			const PolyID      shaderID   = ShaderManager::CreateShader(shaderPath, shaderStage);
			const ShaderData& shaderData = ShaderManager::GetShader(shaderID);

			if (shaderStage == FShaderStage::VERTEX)
				desc.pVertexShader = shaderData.pShader.get();
			else if (shaderStage == FShaderStage::FRAGMENT)
				desc.pFragmentShader = shaderData.pShader.get();
		}

		Ref<GraphicsPipeline> pPipeline = RenderAPI::CreateGraphicsPipeline(&desc);
		return {pLayout, m_Pipelines.emplace(std::move(key), std::move(pPipeline)).first->second.get()};
	}

	void PipelineCache::Clear()
	{
		m_Pipelines.clear();
		m_Layouts.clear();
	}

	PipelineLayout* PipelineCache::GetOrCreateLayout(const ResolvedPass& pass)
	{
		auto it = m_Layouts.find(pass.Name);
		if (it != m_Layouts.end())
			return it->second.get();

		PipelineLayoutDesc desc   = {};
		desc.DescriptorSetLayouts = {ResourceManager::GetSetLayoutDesc()}; // set 0 - shared bindless heap

		if (pass.PushConstantSize > 0)
		{
			PushConstantRange range = {};
			range.ShaderStage       = FShaderStage::VERTEX | FShaderStage::FRAGMENT;
			range.Offset            = 0;
			range.Size              = pass.PushConstantSize;
			desc.PushConstantRanges.push_back(range);
		}

		return m_Layouts.emplace(pass.Name, RenderAPI::CreatePipelineLayout(&desc)).first->second.get();
	}
} // namespace Poly
