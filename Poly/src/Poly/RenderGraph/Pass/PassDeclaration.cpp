#include "PassDeclaration.h"

#include "Poly/RenderGraph/VertexLayoutRegistry.h"

namespace Poly
{
	PassDeclaration::PassDeclaration(std::string_view name)
	{
		m_Desc.Name = name;
	}

	void PassDeclaration::SetDesc(PassDesc desc)
	{
		POLY_VALIDATE(desc.Name == m_Desc.Name, "Description of pass '{}' is for pass '{}'", m_Desc.Name, desc.Name);
		m_Desc      = std::move(desc);
		m_ExecuteFn = nullptr;
	}

	PassDeclaration& PassDeclaration::WithShader(std::string_view shaderPath, FShaderStage stage)
	{
		m_Desc.Shaders.push_back({std::string(shaderPath), stage});
		return *this;
	}

	PassDeclaration& PassDeclaration::WithExecuteFn(std::function<void(ExecuteContext&)> executeFn)
	{
		m_ExecuteFn     = std::move(executeFn);
		m_Desc.Executor = {};
		return *this;
	}

	PassDeclaration& PassDeclaration::WithExecutor(std::string_view executorName, std::string_view paramsJson)
	{
		m_Desc.Executor = {std::string(executorName), std::string(paramsJson)};
		m_ExecuteFn     = nullptr;
		return *this;
	}

	PassDeclaration& PassDeclaration::OnQueue(FQueueType queue)
	{
		m_Desc.Queue = queue;
		return *this;
	}

	PassDeclaration& PassDeclaration::WithTopology(ETopology topology)
	{
		m_Desc.Pipeline.Topology = topology;
		return *this;
	}

	PassDeclaration& PassDeclaration::WithPolygonMode(EPolygonMode mode)
	{
		m_Desc.Pipeline.PolygonMode = mode;
		return *this;
	}

	PassDeclaration& PassDeclaration::WithCullMode(ECullMode mode)
	{
		m_Desc.Pipeline.CullMode = mode;
		return *this;
	}

	PassDeclaration& PassDeclaration::WithFrontFaceClockwise(bool clockwise)
	{
		m_Desc.Pipeline.FrontFaceClockwise = clockwise;
		return *this;
	}

	PassDeclaration& PassDeclaration::WithDepthTest(bool enable)
	{
		m_Desc.Pipeline.DepthTest = enable;
		return *this;
	}

	PassDeclaration& PassDeclaration::WithDepthWrite(bool enable)
	{
		m_Desc.Pipeline.DepthWrite = enable;
		return *this;
	}

	PassDeclaration& PassDeclaration::WithDepthCompareOp(ECompareOp op)
	{
		m_Desc.Pipeline.DepthCompareOp = op;
		return *this;
	}

	PassDeclaration& PassDeclaration::WithDepthBias(float constantFactor, float slopeFactor, float clamp)
	{
		m_Desc.Pipeline.DepthBias = PassDepthBias{.ConstantFactor = constantFactor, .SlopeFactor = slopeFactor, .Clamp = clamp};
		return *this;
	}

	PassDeclaration& PassDeclaration::WithBlend(EBlendMode mode)
	{
		m_Desc.Pipeline.BlendMode = mode;
		m_Desc.Pipeline.CustomBlend.reset();
		return *this;
	}

	PassDeclaration& PassDeclaration::WithBlend(const ColorBlendAttachmentDesc& blend)
	{
		m_Desc.Pipeline.CustomBlend = blend;
		m_Desc.Pipeline.BlendMode.reset();
		return *this;
	}

	PassDeclaration& PassDeclaration::WithVertexLayout(std::string_view layoutName)
	{
		m_Desc.VertexLayoutName = layoutName;
		return *this;
	}

	PassDeclaration& PassDeclaration::WithStencil(const StencilOpStateDesc& state)
	{
		return WithStencil(state, state);
	}

	PassDeclaration& PassDeclaration::WithStencil(const StencilOpStateDesc& front, const StencilOpStateDesc& back)
	{
		m_Desc.Pipeline.Stencil = PassStencilState{.Front = front, .Back = back};
		return *this;
	}

	PassDeclaration& PassDeclaration::WithLogicOp(ELogicOp op)
	{
		m_Desc.Pipeline.LogicOp = op;
		return *this;
	}

	PassDeclaration& PassDeclaration::MapResource(EFeaturePort port, std::string_view shaderResourceName, ELoadOp loadOp)
	{
		m_Desc.ResourceMappings.push_back({port, std::string(shaderResourceName), loadOp});
		return *this;
	}

	PassDeclaration& PassDeclaration::MapGlobal(std::string_view globalName, std::string_view shaderGlobalName)
	{
		m_Desc.GlobalMappings.push_back({std::string(globalName), std::string(shaderGlobalName)});
		return *this;
	}

	PassDeclaration& PassDeclaration::ImportResource(std::string_view resourceName, std::string_view shaderResourceName)
	{
		m_Desc.ImportedResources.push_back({std::string(resourceName), std::string(shaderResourceName)});
		return *this;
	}

	PassDeclaration& PassDeclaration::ExportResource(std::string_view resourceName, std::string_view shaderResourceName)
	{
		m_Desc.ExportedResources.push_back({std::string(resourceName), std::string(shaderResourceName)});
		return *this;
	}

	PassDeclaration& PassDeclaration::ReadResource(std::string_view resourceName, FResourceState state)
	{
		m_Desc.ResourceUses.push_back({std::string(resourceName), state, false});
		return *this;
	}

	PassDeclaration& PassDeclaration::WriteResource(std::string_view resourceName, FResourceState state)
	{
		m_Desc.ResourceUses.push_back({std::string(resourceName), state, true});
		return *this;
	}

	GraphicsPipelineDesc PassDeclaration::BuildPipelineDesc(const VertexLayout* pVertexLayout) const
	{
		const PassPipelineState& state = m_Desc.Pipeline;

		GraphicsPipelineDesc desc = {};

		if (pVertexLayout)
		{
			for (const VertexAttribute& attribute : pVertexLayout->Attributes)
			{
				desc.VertexInputs.push_back({.Stride          = pVertexLayout->Stride,
				                             .VertexInputRate = pVertexLayout->InputRate,
				                             .Location        = attribute.Location,
				                             .Format          = attribute.Format,
				                             .Offset          = attribute.Offset});
			}
		}

		if (state.Topology)
			desc.InputAssembly.Topology = *state.Topology;
		if (state.PolygonMode)
			desc.Rasterization.PolygonMode = *state.PolygonMode;
		if (state.CullMode)
			desc.Rasterization.CullMode = *state.CullMode;
		if (state.FrontFaceClockwise)
			desc.Rasterization.ClockwiseFrontFace = *state.FrontFaceClockwise;

		if (state.DepthBias)
		{
			desc.Rasterization.DepthBiasEnable      = true;
			desc.Rasterization.DepthBiasFactor      = state.DepthBias->ConstantFactor;
			desc.Rasterization.DepthBiasSlopeFactor = state.DepthBias->SlopeFactor;
			desc.Rasterization.DepthBiasClamp       = state.DepthBias->Clamp;
		}

		uint32 colorOutputCount = 0;
		bool   hasDepth         = false;
		for (const ResourceMapping& mapping : m_Desc.ResourceMappings)
		{
			if (mapping.Port == EFeaturePort::Color)
				colorOutputCount++;
			else if (mapping.Port == EFeaturePort::Depth)
				hasDepth = true;
		}

		desc.DepthStencil.DepthTestEnable  = state.DepthTest.value_or(hasDepth);
		desc.DepthStencil.DepthWriteEnable = state.DepthWrite.value_or(desc.DepthStencil.DepthTestEnable);
		if (state.DepthCompareOp)
			desc.DepthStencil.DepthCompareOp = *state.DepthCompareOp;

		if (state.Stencil)
		{
			desc.DepthStencil.StencilTestEnable = true;
			desc.DepthStencil.Front             = state.Stencil->Front;
			desc.DepthStencil.Back              = state.Stencil->Back;
		}

		const ColorBlendAttachmentDesc blend = state.CustomBlend ? *state.CustomBlend : ToBlendDesc(state.BlendMode.value_or(EBlendMode::Opaque));
		desc.ColorBlendState.ColorBlendAttachments.assign(colorOutputCount, blend);

		if (state.LogicOp)
		{
			desc.ColorBlendState.LogicOpEnable = true;
			desc.ColorBlendState.LogicOp       = *state.LogicOp;
		}

		return desc;
	}
} // namespace Poly
