#include "RGCDebugLogger.h"

#include "polypch.h"
#include "RGCContext.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCSyncTypes.h"
#include "Poly/Rendering/RenderGraph/EdgeData.h"
#include "Poly/Rendering/RenderGraph/Pass.h"
#include "Poly/Rendering/RenderGraph/Reflection/FieldVisibility.h"
#include "Poly/Rendering/RenderGraph/Reflection/PassField.h"
#include "Poly/Rendering/RenderGraph/RenderGraph.h"
#include "Poly/Rendering/RenderGraph/RenderPass.h"
#include "Poly/Rendering/RenderGraph/SyncPass.h"

namespace Poly
{
	// ---------------------------------------------------------------------------
	// Enum-to-string helpers (local to this translation unit)
	// ---------------------------------------------------------------------------

	static const char* ToString(ETextureLayout layout)
	{
		switch (layout)
		{
			case ETextureLayout::UNDEFINED:                                  return "UNDEFINED";
			case ETextureLayout::GENERAL:                                    return "GENERAL";
			case ETextureLayout::COLOR_ATTACHMENT_OPTIMAL:                   return "COLOR_ATTACHMENT_OPTIMAL";
			case ETextureLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL:           return "DEPTH_STENCIL_ATTACHMENT_OPTIMAL";
			case ETextureLayout::DEPTH_STENCIL_READ_ONLY_OPTIMAL:            return "DEPTH_STENCIL_READ_ONLY_OPTIMAL";
			case ETextureLayout::SHADER_READ_ONLY_OPTIMAL:                   return "SHADER_READ_ONLY_OPTIMAL";
			case ETextureLayout::TRANSFER_SRC_OPTIMAL:                       return "TRANSFER_SRC_OPTIMAL";
			case ETextureLayout::TRANSFER_DST_OPTIMAL:                       return "TRANSFER_DST_OPTIMAL";
			case ETextureLayout::PREINITIALIZED:                             return "PREINITIALIZED";
			case ETextureLayout::DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL: return "DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL";
			case ETextureLayout::DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL: return "DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL";
			case ETextureLayout::DEPTH_ATTACHMENT_OPTIMAL:                   return "DEPTH_ATTACHMENT_OPTIMAL";
			case ETextureLayout::DEPTH_READ_ONLY_OPTIMAL:                    return "DEPTH_READ_ONLY_OPTIMAL";
			case ETextureLayout::STENCIL_ATTACHMENT_OPTIMAL:                 return "STENCIL_ATTACHMENT_OPTIMAL";
			case ETextureLayout::STENCIL_READ_ONLY_OPTIMAL:                  return "STENCIL_READ_ONLY_OPTIMAL";
			case ETextureLayout::PRESENT:                                    return "PRESENT";
			case ETextureLayout::SHARED_PRESENT:                             return "SHARED_PRESENT";
			default:                                                         return "UNKNOWN";
		}
	}

	static std::string ToString(FAccessFlag flags)
	{
		if (flags == FAccessFlag::NONE)
			return "NONE";

		std::string result;
		auto        append = [&](FAccessFlag flag, const char* name)
		{
			if (BitsSet(flags, flag))
			{
				if (!result.empty())
					result += '|';
				result += name;
			}
		};
		append(FAccessFlag::INDIRECT_COMMAND_READ,          "INDIRECT_COMMAND_READ");
		append(FAccessFlag::INDEX_READ,                     "INDEX_READ");
		append(FAccessFlag::VERTEX_ATTRIBUTE_READ,          "VERTEX_ATTRIBUTE_READ");
		append(FAccessFlag::UNIFORM_READ,                   "UNIFORM_READ");
		append(FAccessFlag::INPUT_ATTACHMENT_READ,          "INPUT_ATTACHMENT_READ");
		append(FAccessFlag::SHADER_READ,                    "SHADER_READ");
		append(FAccessFlag::SHADER_WRITE,                   "SHADER_WRITE");
		append(FAccessFlag::COLOR_ATTACHMENT_READ,          "COLOR_ATTACHMENT_READ");
		append(FAccessFlag::COLOR_ATTACHMENT_WRITE,         "COLOR_ATTACHMENT_WRITE");
		append(FAccessFlag::DEPTH_STENCIL_ATTACHMENT_READ,  "DEPTH_STENCIL_ATTACHMENT_READ");
		append(FAccessFlag::DEPTH_STENCIL_ATTACHMENT_WRITE, "DEPTH_STENCIL_ATTACHMENT_WRITE");
		append(FAccessFlag::TRANSFER_READ,                  "TRANSFER_READ");
		append(FAccessFlag::TRANSFER_WRITE,                 "TRANSFER_WRITE");
		append(FAccessFlag::HOST_READ,                      "HOST_READ");
		append(FAccessFlag::HOST_WRITE,                     "HOST_WRITE");
		append(FAccessFlag::MEMORY_READ,                    "MEMORY_READ");
		append(FAccessFlag::MEMORY_WRITE,                   "MEMORY_WRITE");
		return result;
	}

	static std::string ToString(FPipelineStage stages)
	{
		if (stages == FPipelineStage::NONE)
			return "NONE";

		std::string result;
		auto        append = [&](FPipelineStage stage, const char* name)
		{
			if (BitsSet(stages, stage))
			{
				if (!result.empty())
					result += '|';
				result += name;
			}
		};
		append(FPipelineStage::TOP_OF_PIPE,             "TOP_OF_PIPE");
		append(FPipelineStage::DRAW_INDIRECT,           "DRAW_INDIRECT");
		append(FPipelineStage::VERTEX_INPUT,            "VERTEX_INPUT");
		append(FPipelineStage::VERTEX_SHADER,           "VERTEX_SHADER");
		append(FPipelineStage::FRAGMENT_SHADER,         "FRAGMENT_SHADER");
		append(FPipelineStage::EARLY_FRAGMENT_TEST,     "EARLY_FRAGMENT_TEST");
		append(FPipelineStage::LATE_FRAGMENT_TEST,      "LATE_FRAGMENT_TEST");
		append(FPipelineStage::COLOR_ATTACHMENT_OUTPUT, "COLOR_ATTACHMENT_OUTPUT");
		append(FPipelineStage::COMPUTE_SHADER,          "COMPUTE_SHADER");
		append(FPipelineStage::TRANSFER,                "TRANSFER");
		append(FPipelineStage::BOTTOM_OF_PIPE,          "BOTTOM_OF_PIPE");
		append(FPipelineStage::ALL_GRAPHICS,            "ALL_GRAPHICS");
		append(FPipelineStage::ALL_COMMANDS,            "ALL_COMMANDS");
		append(FPipelineStage::HOST,                    "HOST");
		return result;
	}

	static const char* ToString(SyncPass::SyncType type)
	{
		switch (type)
		{
			case SyncPass::SyncType::TEXTURE: return "TEXTURE";
			case SyncPass::SyncType::BUFFER:  return "BUFFER";
			case SyncPass::SyncType::MEMORY:  return "MEMORY";
			default:                          return "NONE";
		}
	}

	static const char* ToString(Pass::Type type)
	{
		switch (type)
		{
			case Pass::Type::RENDER:   return "RENDER";
			case Pass::Type::COMPUTE:  return "COMPUTE";
			case Pass::Type::SYNC:     return "SYNC";
			case Pass::Type::TRANSFER: return "TRANSFER";
			case Pass::Type::EXTERNAL: return "EXTERNAL";
			default:                   return "NONE";
		}
	}

	static const char* ToString(PassField::EType type)
	{
		switch (type)
		{
			case PassField::EType::Texture: return "Texture";
			case PassField::EType::Buffer:  return "Buffer";
			default:                        return "None";
		}
	}

	// ---------------------------------------------------------------------------
	// Execute
	// ---------------------------------------------------------------------------

	void RGCDebugLogger::Execute(const RGCContext& ctx)
	{
		POLY_CORE_TRACE("==========================================================");
		POLY_CORE_TRACE("RenderGraph Compiled Graph: \"{}\"", ctx.RenderGraph.m_Name);
		POLY_CORE_TRACE("  Passes: {}", ctx.CompiledGraph.CompiledPasses.size());
		POLY_CORE_TRACE("==========================================================");

		// ----------------------------------------------------------------
		// Passes in execution order
		// ----------------------------------------------------------------
		for (const auto& compiledPass : ctx.CompiledGraph.CompiledPasses)
		{
			const Pass*        pPass     = compiledPass.pPass.get();
			const std::string& passName  = pPass->GetName();
			const uint32       nodeIndex = compiledPass.GraphNodeIndex;

			POLY_CORE_TRACE("----------------------------------------------------------");
			POLY_CORE_TRACE("Pass [{}]  \"{}\"  type: {}  execIdx: {}  nodeIdx: {}",
			    compiledPass.ExecutionIndex,
			    passName,
			    ToString(pPass->GetPassType()),
			    compiledPass.ExecutionIndex,
			    nodeIndex);

			// ---- Incoming connections ----
			const auto* pNode = ctx.RenderGraph.m_pGraph->GetNode(nodeIndex);
			if (pNode)
			{
				const auto& incomingEdges = pNode->GetIncommingEdges();
				if (!incomingEdges.empty())
				{
					POLY_CORE_TRACE("  Incoming connections:");
					for (uint32 edgeID : incomingEdges)
					{
						const EdgeData& edge = ctx.RenderGraph.m_Edges.at(edgeID);
						if (edge.IsPassToPass())
						{
							POLY_CORE_TRACE("    [exec-dep]  {} --> {}",
							    edge.GetSrcPass().GetName(),
							    edge.GetDstPass().GetName());
						}
						else if (edge.IsExternalResourceToPassResource())
						{
							POLY_CORE_TRACE("    [external]  $.{} --> {}.{}",
							    edge.GetSrcExternalResource().GetName(),
							    edge.GetDstPassRes().GetPass().GetName(),
							    edge.GetDstPassRes().GetResource().GetName());
						}
						else if (edge.IsPassResourceToPassResource())
						{
							POLY_CORE_TRACE("    [data]  {}.{} --> {}.{}",
							    edge.GetSrcPassRes().GetPass().GetName(),
							    edge.GetSrcPassRes().GetResource().GetName(),
							    edge.GetDstPassRes().GetPass().GetName(),
							    edge.GetDstPassRes().GetResource().GetName());
						}
					}
				}
				else
				{
					POLY_CORE_TRACE("  Incoming connections: none");
				}
			}

			// ---- Pass-type-specific data ----
			if (pPass->GetPassType() == Pass::Type::RENDER)
			{
				auto* pRenderPass = static_cast<RenderPass*>(compiledPass.pPass.get());
				const auto& attachments = pRenderPass->GetAttachments();
				POLY_CORE_TRACE("  Attachments ({}):", attachments.size());
				for (const auto& att : attachments)
				{
					POLY_CORE_TRACE("    [{}] \"{}\"  initial: {}  used: {}  final: {}",
					    att.Index,
					    att.Name,
					    ToString(att.InitalLayout),
					    ToString(att.UsedLayout),
					    ToString(att.FinalLayout));
				}
				POLY_CORE_TRACE("  DepthStencil: {}", pRenderPass->GetDepthStenctilUse() ? "yes" : "no");
			}
			else if (pPass->GetPassType() == Pass::Type::SYNC)
			{
				auto*      pSyncPass = static_cast<SyncPass*>(compiledPass.pPass.get());
				const auto syncData  = pSyncPass->GetSyncData();
				POLY_CORE_TRACE("  Sync barriers ({}):", syncData.size());
				for (const auto& data : syncData)
				{
					POLY_CORE_TRACE("    resource: \"{}\"  type: {}",
					    data.ResourceName,
					    ToString(data.Type));
					POLY_CORE_TRACE("      layout:  {} --> {}",
					    ToString(data.SrcLayout),
					    ToString(data.DstLayout));
					POLY_CORE_TRACE("      access:  {} --> {}",
					    ToString(data.SrcAccessFlag),
					    ToString(data.DstAccessFlag));
					POLY_CORE_TRACE("      stage:   {} --> {}",
					    ToString(data.SrcPipelineStage),
					    ToString(data.DstPipelineStage));
				}
			}

			// ---- Reflection fields ----
			{
				auto inputs  = compiledPass.Reflection.GetFields(FFieldVisibility::INPUT);
				auto outputs = compiledPass.Reflection.GetFields(FFieldVisibility::OUTPUT);

				if (!inputs.empty())
				{
					POLY_CORE_TRACE("  Inputs ({}):", inputs.size());
					for (const PassField* pField : inputs)
					{
						POLY_CORE_TRACE("    \"{}\"  type: {}  layout: {}",
						    pField->GetName(),
						    ToString(pField->GetType()),
						    ToString(pField->GetTextureLayout()));
					}
				}

				if (!outputs.empty())
				{
					POLY_CORE_TRACE("  Outputs ({}):", outputs.size());
					for (const PassField* pField : outputs)
					{
						POLY_CORE_TRACE("    \"{}\"  type: {}  layout: {}",
						    pField->GetName(),
						    ToString(pField->GetType()),
						    ToString(pField->GetTextureLayout()));
					}
				}
			}
		}

		// ----------------------------------------------------------------
		// Post-sync resource states
		// ----------------------------------------------------------------
		POLY_CORE_TRACE("----------------------------------------------------------");
		POLY_CORE_TRACE("PostSync resource states ({}):", ctx.PostSyncResourceStates.size());
		for (const auto& [passResID, state] : ctx.PostSyncResourceStates)
		{
			POLY_CORE_TRACE("  {}  layout: {}  access: {}  stage: {}",
			    passResID.GetFullName(),
			    ToString(state.Layout),
			    ToString(state.AccessMask),
			    ToString(state.Stage));
		}

		// ----------------------------------------------------------------
		// Debug texture GUIDs
		// ----------------------------------------------------------------
		if (!ctx.DebugTextureGUIDs.empty())
		{
			POLY_CORE_TRACE("----------------------------------------------------------");
			POLY_CORE_TRACE("Debug texture GUIDs ({}):", ctx.DebugTextureGUIDs.size());
			for (const auto& guid : ctx.DebugTextureGUIDs)
				POLY_CORE_TRACE("  {}", guid.GetFullName());
		}

		POLY_CORE_TRACE("==========================================================");
	}
} // namespace Poly
