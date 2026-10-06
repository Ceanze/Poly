#include "PassDeclaration.h"

#include "Poly/Resources/PathUtils.h"

namespace Poly
{
	PassDeclaration::PassDeclaration(std::string_view name)
	    : m_Name(name)
	{}

	PassDeclaration& PassDeclaration::WithShader(std::string_view shaderPath, FShaderStage stage)
	{
		m_Shaders.emplace_back(std::string(shaderPath), stage);
		return *this;
	}

	PassDeclaration& PassDeclaration::WithShader(std::string_view shaderPath)
	{
		const std::string extension = PathUtils::GetExtension(shaderPath);

		if (extension == "vert")
			return WithShader(shaderPath, FShaderStage::VERTEX);
		if (extension == "frag")
			return WithShader(shaderPath, FShaderStage::FRAGMENT);
		if (extension == "comp")
			return WithShader(shaderPath, FShaderStage::COMPUTE);

		POLY_CORE_ERROR("Pass '{}': cannot tell the stage of shader '{}' from its extension, pass the stage explicitly; skipping.", m_Name, shaderPath);
		return *this;
	}

	PassDeclaration& PassDeclaration::WithSetupFn(std::function<void(SetupContext&)> setupFn)
	{
		m_SetupFn = std::move(setupFn);
		return *this;
	}

	PassDeclaration& PassDeclaration::WithExecuteFn(std::function<void(ExecuteContext&)> executeFn)
	{
		m_ExecuteFn = std::move(executeFn);
		return *this;
	}

	PassDeclaration& PassDeclaration::OnQueue(FQueueType queue)
	{
		m_Queue = queue;
		return *this;
	}

	PassDeclaration& PassDeclaration::WithTopology(ETopology topology)
	{
		m_PipelineDesc.InputAssembly.Topology = topology;
		return *this;
	}

	PassDeclaration& PassDeclaration::WithPolygonMode(EPolygonMode mode)
	{
		m_PipelineDesc.Rasterization.PolygonMode = mode;
		return *this;
	}

	PassDeclaration& PassDeclaration::WithCullMode(ECullMode mode)
	{
		m_PipelineDesc.Rasterization.CullMode = mode;
		return *this;
	}

	PassDeclaration& PassDeclaration::WithFrontFaceClockwise(bool clockwise)
	{
		m_PipelineDesc.Rasterization.ClockwiseFrontFace = clockwise;
		return *this;
	}

	PassDeclaration& PassDeclaration::WithDepthTest(bool enable)
	{
		m_DepthTest = enable;
		return *this;
	}

	PassDeclaration& PassDeclaration::WithDepthWrite(bool enable)
	{
		m_DepthWrite = enable;
		return *this;
	}

	PassDeclaration& PassDeclaration::WithDepthCompareOp(ECompareOp op)
	{
		m_PipelineDesc.DepthStencil.DepthCompareOp = op;
		return *this;
	}

	PassDeclaration& PassDeclaration::WithDepthBias(float constantFactor, float slopeFactor, float clamp)
	{
		m_PipelineDesc.Rasterization.DepthBiasEnable      = true;
		m_PipelineDesc.Rasterization.DepthBiasFactor      = constantFactor;
		m_PipelineDesc.Rasterization.DepthBiasSlopeFactor = slopeFactor;
		m_PipelineDesc.Rasterization.DepthBiasClamp       = clamp;
		return *this;
	}

	PassDeclaration& PassDeclaration::WithBlend(EBlendMode mode)
	{
		m_Blend = ToBlendDesc(mode);
		return *this;
	}

	PassDeclaration& PassDeclaration::WithBlend(const ColorBlendAttachmentDesc& blend)
	{
		m_Blend = blend;
		return *this;
	}

	PassDeclaration& PassDeclaration::WithVertexLayout(uint32 stride, EVertexInputRate inputRate)
	{
		m_VertexStride    = stride;
		m_VertexInputRate = inputRate;
		return *this;
	}

	PassDeclaration& PassDeclaration::WithVertexAttribute(uint32 location, EFormat format, uint32 offset)
	{
		m_PipelineDesc.VertexInputs.push_back({.Location = location, .Format = format, .Offset = offset});
		return *this;
	}

	PassDeclaration& PassDeclaration::WithPipelineOverride(std::function<void(GraphicsPipelineDesc&)> overrideFn)
	{
		m_PipelineOverrideFn = std::move(overrideFn);
		return *this;
	}

	PassDeclaration& PassDeclaration::MapResource(EFeaturePort port, std::string_view shaderResourceName, ELoadOp loadOp)
	{
		m_ResourceMappings.push_back({port, std::string(shaderResourceName), loadOp});
		return *this;
	}

	PassDeclaration& PassDeclaration::MapGlobal(std::string_view globalName, std::string_view shaderGlobalName)
	{
		m_GlobalMappings.emplace_back(std::string(globalName), std::string(shaderGlobalName));
		return *this;
	}

	PassDeclaration& PassDeclaration::ImportResource(std::string_view resourceName, std::string_view shaderResourceName)
	{
		m_ImportedResources.emplace_back(std::string(resourceName), std::string(shaderResourceName));
		return *this;
	}

	PassDeclaration& PassDeclaration::ExportResource(std::string_view resourceName, std::string_view shaderResourceName)
	{
		m_ExportedResources.emplace_back(std::string(resourceName), std::string(shaderResourceName));
		return *this;
	}

	PassDeclaration& PassDeclaration::ReadResource(std::string_view resourceName, FResourceState state)
	{
		m_ResourceUses.push_back({std::string(resourceName), state, false});
		return *this;
	}

	PassDeclaration& PassDeclaration::WriteResource(std::string_view resourceName, FResourceState state)
	{
		m_ResourceUses.push_back({std::string(resourceName), state, true});
		return *this;
	}

	GraphicsPipelineDesc PassDeclaration::BuildPipelineDesc() const
	{
		GraphicsPipelineDesc desc = m_PipelineDesc;

		// The backend has a single vertex binding, shared by every attribute
		if (!desc.VertexInputs.empty() && m_VertexStride == 0)
			POLY_CORE_ERROR("Pass '{}' declares vertex attributes without a vertex layout, call WithVertexLayout().", m_Name);

		for (VertexInput& vertexInput : desc.VertexInputs)
		{
			vertexInput.Stride          = m_VertexStride;
			vertexInput.VertexInputRate = m_VertexInputRate;
		}

		uint32 colorOutputCount = 0;
		bool   hasDepth         = false;
		for (const ResourceMapping& mapping : m_ResourceMappings)
		{
			if (mapping.Port == EFeaturePort::Color)
				colorOutputCount++;
			else if (mapping.Port == EFeaturePort::Depth)
				hasDepth = true;
		}

		desc.DepthStencil.DepthTestEnable  = m_DepthTest.value_or(hasDepth);
		desc.DepthStencil.DepthWriteEnable = m_DepthWrite.value_or(desc.DepthStencil.DepthTestEnable);
		desc.ColorBlendState.ColorBlendAttachments.assign(colorOutputCount, m_Blend);

		if (m_PipelineOverrideFn)
			m_PipelineOverrideFn(desc);

		return desc;
	}

	void PassDeclaration::CallSetupFn(SetupContext& ctx) const
	{
		if (m_SetupFn)
			m_SetupFn(ctx);
	}

	void PassDeclaration::CallExecuteFn(ExecuteContext& ctx) const
	{
		if (m_ExecuteFn)
			m_ExecuteFn(ctx);
	}

} // namespace Poly
