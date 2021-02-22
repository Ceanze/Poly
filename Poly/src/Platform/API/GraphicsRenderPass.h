#pragma once

#include <vector>

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	struct GraphicsRenderPassAttachmentDesc
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

	struct GraphicsRenderPassSubpassDependencyDesc
	{
		uint32			SrcSubpass		= EXTERNAL_SUBPASS;
		uint32			DstSubpass		= EXTERNAL_SUBPASS;
		FPipelineStage	SrcStageMask	= FPipelineStage::NONE;
		FPipelineStage	DstStageMask	= FPipelineStage::NONE;
		FAccessFlag		SrcAccessMask	= FAccessFlag::NONE;
		FAccessFlag		DstAccessMask	= FAccessFlag::NONE;
	};

	struct GraphicsRenderPassSubpassAttachmentReference
	{
		uint32			Index = 0;
		ETextureLayout	Layout = ETextureLayout::UNDEFINED;
	};

	struct GraphicsRenderPassSubpassDesc
	{
		std::vector<GraphicsRenderPassSubpassAttachmentReference>	InputAttachmentsLayouts;
		std::vector<GraphicsRenderPassSubpassAttachmentReference>	ColorAttachmentsLayouts;
		std::vector<GraphicsRenderPassSubpassAttachmentReference>	ResolveAttachmentsLayouts;
		GraphicsRenderPassSubpassAttachmentReference				DepthStencilAttachmentLayout;
	};

	struct GraphicsRenderPassDesc
	{
		std::vector<GraphicsRenderPassAttachmentDesc>			Attachments;
		std::vector<GraphicsRenderPassSubpassDesc>				Subpasses;
		std::vector<GraphicsRenderPassSubpassDependencyDesc>	SubpassDependencies;
	};

	class GraphicsRenderPass
	{
	public:
		CLASS_ABSTRACT(GraphicsRenderPass);

		/**
		 * Init the Buffer object
		 * @param desc	Buffer creation description
		*/
		virtual void Init(const GraphicsRenderPassDesc* pDesc) = 0;

		/**
		 * @return Native handle to the API specific object
		 */
		virtual uint64 GetNative() const = 0;

	protected:
		GraphicsRenderPassDesc p_PipelineDesc;
	};
}