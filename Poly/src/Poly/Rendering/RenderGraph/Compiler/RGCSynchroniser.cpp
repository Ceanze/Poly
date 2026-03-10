#include "Poly/Rendering/RenderGraph/Compiler/RGCSynchroniser.h"

#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Poly/Rendering/RenderGraph/Reflection/PassField.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCContext.h"
#include "Poly/Rendering/RenderGraph/Pass.h"
#include "Poly/Rendering/RenderGraph/RenderPass.h"
#include "Poly/Rendering/RenderGraph/ResourceCache.h"

namespace Poly
{
	FAccessFlag GetAccessFlag(FResourceBindPoint bindPoint, bool isReadOnly)
	{
		if (BitsSet(bindPoint, FResourceBindPoint::COLOR_ATTACHMENT))
			return isReadOnly ? FAccessFlag::COLOR_ATTACHMENT_READ : FAccessFlag::COLOR_ATTACHMENT_WRITE;
		if (BitsSet(bindPoint, FResourceBindPoint::DEPTH_STENCIL))
			return isReadOnly ? FAccessFlag::DEPTH_STENCIL_ATTACHMENT_READ : FAccessFlag::DEPTH_STENCIL_ATTACHMENT_WRITE;
		if (BitsSet(bindPoint, FResourceBindPoint::UNIFORM))
			return FAccessFlag::UNIFORM_READ;
		if (BitsSet(bindPoint, FResourceBindPoint::SAMPLER))
			return FAccessFlag::SHADER_READ;
		if (BitsSet(bindPoint, FResourceBindPoint::STORAGE))
			return isReadOnly ? FAccessFlag::SHADER_READ : FAccessFlag::SHADER_WRITE;
		if (BitsSet(bindPoint, FResourceBindPoint::VERTEX))
			return FAccessFlag::VERTEX_ATTRIBUTE_READ;
		if (BitsSet(bindPoint, FResourceBindPoint::INDEX))
			return FAccessFlag::INDEX_READ;
		if (BitsSet(bindPoint, FResourceBindPoint::INDIRECT))
			return FAccessFlag::INDIRECT_COMMAND_READ;
		if (BitsSet(bindPoint, FResourceBindPoint::INPUT_ATTACHMENT))
			return FAccessFlag::INPUT_ATTACHMENT_READ;
		if (BitsSet(bindPoint, FResourceBindPoint::EXTERNAL))
			return FAccessFlag::SHADER_READ;

		return FAccessFlag::NONE;
	}

	FPipelineStage GetPipelineStage(FResourceBindPoint bindPoint)
	{
		if (BitsSet(bindPoint, FResourceBindPoint::COLOR_ATTACHMENT))
			return FPipelineStage::COLOR_ATTACHMENT_OUTPUT;
		if (BitsSet(bindPoint, FResourceBindPoint::DEPTH_STENCIL))
			return FPipelineStage::LATE_FRAGMENT_TEST;
		if (BitsSet(bindPoint, FResourceBindPoint::UNIFORM))
			return FPipelineStage::VERTEX_SHADER | FPipelineStage::FRAGMENT_SHADER;
		if (BitsSet(bindPoint, FResourceBindPoint::SAMPLER))
			return FPipelineStage::FRAGMENT_SHADER;
		if (BitsSet(bindPoint, FResourceBindPoint::STORAGE))
			return FPipelineStage::ALL_GRAPHICS; // Or COMPUTE_SHADER when compute is added
		if (BitsSet(bindPoint, FResourceBindPoint::VERTEX))
			return FPipelineStage::VERTEX_SHADER;
		if (BitsSet(bindPoint, FResourceBindPoint::INDEX))
			return FPipelineStage::BOTTOM_OF_PIPE;
		if (BitsSet(bindPoint, FResourceBindPoint::INDIRECT))
			return FPipelineStage::DRAW_INDIRECT;
		if (BitsSet(bindPoint, FResourceBindPoint::INPUT_ATTACHMENT))
			return FPipelineStage::VERTEX_INPUT;
		if (BitsSet(bindPoint, FResourceBindPoint::EXTERNAL))
			return FPipelineStage::FRAGMENT_SHADER;

		return FPipelineStage::NONE;
	}

	enum class EAccessType
	{
		None,
		Read,
		Write,
		ReadWrite
	};

	// Records how a resource is being used 
	struct ResourceUsage
	{
		std::string			Name			= "";
		PassField::EType	Type			= PassField::EType::None;
		ETextureLayout		TextureLayout	= ETextureLayout::UNDEFINED;
		FPipelineStage		PipelineStage	= FPipelineStage::NONE;
		FAccessFlag			AccessMask		= FAccessFlag::NONE;
		EAccessType			AccessType		= EAccessType::None;
	};

	// Records the current state of a resource
	struct ResourceState
	{
		ETextureLayout	TextureLayout	= ETextureLayout::UNDEFINED;
		EAccessType		AccessType		= EAccessType::None;
	};

	struct SyncContext
	{
		std::unordered_map<uint32, std::vector<ResourceUsage>> PassResourceUsages;
		std::unordered_map<uint32, ResourceState> CurrentResourceState;
	};

	void RGCSynchroniser::Execute(RGCContext& ctx)
	{
		// - For each pass
		//   - For each resource
		//     - Save resource usage (AccesMask, PipelineStage, TextureLayout, Read/Write behaviour)
		//     - Set the lastUsage state
		//     - Setup attachment usage (add and intial to undefined), if any
		// - For each pass
		//   - For each resource usage
		//     - Go through incoming edges
		//     - Check for WAR, RAW, WAW, and layout for last usage
		//       - Add sync if any writes, set attachment/layouts for pass/sync
		// - For each output connection
		//   - Set final layout to present
		// - For each created sync pass
		//   - Add to render graph, mark as dirty

		SyncContext syncCtx;
	}

	void RGCSynchroniser::SetupResourceUsage(RGCContext& ctx, SyncContext& syncCtx)
	{
		for (const auto& compiledPass : ctx.CompiledGraph.CompiledPasses)
		{
			// Get resources
			auto inputs = compiledPass.Reflection.GetFieldsFiltered(FFieldVisibility::INPUT, FResourceBindPoint::INTERNAL_USE);
			auto outputs = compiledPass.Reflection.GetFieldsFiltered(FFieldVisibility::OUTPUT, FResourceBindPoint::INTERNAL_USE);

			std::move(outputs.begin(), outputs.end(), std::back_inserter(inputs));
			std::sort(inputs.begin(), inputs.end());

			auto& resources = inputs;
			auto last = std::unique(resources.begin(), resources.end());
			resources.erase(last, resources.end());

			uint32 attachmentIndex = 0;
			for (const auto& passResource : resources)
			{
				EAccessType accessType;
				if (BitsSet(passResource->GetVisibility(), FFieldVisibility::IN_OUT))
					accessType = EAccessType::ReadWrite;
				else if (BitsSet(passResource->GetVisibility(), FFieldVisibility::INPUT))
					accessType = EAccessType::Read;
				else
					accessType = EAccessType::Read;

				ResourceUsage usage = {};
				usage.Name			= passResource->GetName();
				usage.AccessMask	= GetAccessFlag(passResource->GetBindPoint(), !BitsSet(passResource->GetVisibility(), FFieldVisibility::OUTPUT));
				usage.AccessType	= accessType;
				usage.PipelineStage	= GetPipelineStage(passResource->GetBindPoint());
				usage.TextureLayout	= passResource->GetTextureLayout();
				usage.Type			= passResource->GetType();

				syncCtx.PassResourceUsages[compiledPass.GraphNodeIndex].emplace_back(usage);

				const bool writes = accessType == EAccessType::Write || accessType == EAccessType::ReadWrite;
				if (compiledPass.pPass->GetPassType() == Pass::Type::RENDER && passResource->GetType() == PassField::EType::Texture && writes)
				{
					// Assume the data will be cleared for now
					static_cast<RenderPass*>(compiledPass.pPass.get())->AddAttachment(passResource->GetName(), passResource->GetTextureLayout(), attachmentIndex++, passResource->GetFormat());
					static_cast<RenderPass*>(compiledPass.pPass.get())->SetAttachmentInital(passResource->GetName(), ETextureLayout::UNDEFINED);
				}

				ResourceGUID resourceGUID(compiledPass.pPass->GetName(), passResource->GetName());
				syncCtx.CurrentResourceState[ctx.pResourceCache->GetResourceIndex(resourceGUID)] = ResourceState{ usage.TextureLayout, accessType };
			}
			
		}
	}
}