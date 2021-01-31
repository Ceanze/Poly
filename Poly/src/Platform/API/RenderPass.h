#pragma once

#include <vector>

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	struct RenderPassAttachmentDesc
	{
		EFormat			Format			= EFormat::UNDEFINED;
		uint32			SampleCount		= 0;
		ELoadOp			LoadOp			= ELoadOp::NONE;
		EStoreOp		StoreOp			= EStoreOp::NONE;
		ELoadOp			StencilLoadOp	= ELoadOp::NONE;
		EStoreOp		StencilStoreOp	= EStoreOp::NONE;
		ETextureLayout	InitialLayout	= ETextureLayout::UNDEFINED;
		ETextureLayout	FinalLayout		= ETextureLayout::UNDEFINED;
	};

	struct RenderPassSubpassDependencyDesc
	{
		uint32			SrcSubpass		= EXTERNAL_SUBPASS;
		uint32			DstSubpass		= EXTERNAL_SUBPASS;
		FPipelineStage	SrcStageMask	= FPipelineStage::NONE;
		FPipelineStage	DstStageMask	= FPipelineStage::NONE;
		FAccessFlag		SrcAccessMask	= FAccessFlag::NONE;
		FAccessFlag		DstAccessMask	= FAccessFlag::NONE;
	};

	struct RenderPassSubpassAttachmentReference
	{
		uint32			Index	= 0;
		ETextureLayout	Layout	= ETextureLayout::UNDEFINED;
	}

	struct RenderPassSubpassDesc
	{
		std::vector<RenderPassSubpassAttachmentReference>	InputAttachmentsLayouts;
		std::vector<RenderPassSubpassAttachmentReference>	ColorAttachmentsLayouts;
		std::vector<RenderPassSubpassAttachmentReference>	ResolveAttachmentsLayouts;
		RenderPassSubpassAttachmentReference				DepthStencilAttachmentLayout;
	};

	struct RenderPassDesc
	{
		std::vector<RenderPassAttachmentDesc>			Attachments;
		std::vector<RenderPassSubpassDesc>				Subpasses;
		std::vector<RenderPassSubpassDependencyDesc>	SubpassDependencies;
	};

	class RenderPass
	{
	public:
		CLASS_ABSTRACT(RenderPass);

		/**
		 * Init the Buffer object
		 * @param desc	Buffer creation description
		*/
		virtual void Init(const RenderPassDesc* pDesc) = 0;

		/**
		 * @return Native handle to the API specific object
		 */
		virtual uint64 GetNative() const = 0;

	protected:
		RenderPassDesc p_PipelineDesc;
	};
}