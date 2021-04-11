#pragma once

#include <vector>

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Platform/API/Pipeline.h"

/**
 * This pipeline is only for Graphics until Compute is added later
 */

namespace Poly
{
	class Shader;
	class PipelineLayout;
	class GraphicsRenderPass;

	struct VertexInput
	{
		uint32				Binding			= 0;
		uint32				Stride			= 0;
		EVertexInputRate	VertexInputRate	= EVertexInputRate::NONE;
		uint32				Location		= 0;
		EFormat				Format			= EFormat::UNDEFINED;
		uint32				Offset			= 0;
	};

	struct InputAssemblyDesc
	{
		ETopology	Topology			= ETopology::NONE;
		bool		RestartPrimitive	= false;
	};

	struct ViewportDesc
	{
		bool	IsDynamic	= true; // True - the other parameters are ignored and set dynamically at render time
		float	PosX		= 0.0f;
		float	PosY		= 0.0f;
		float	Width		= 0.0f;
		float	Height		= 0.0f;
		float	MinDepth	= 0.0f;
		float	MaxDepth	= 1.0f;
	};

	struct ScissorDesc
	{
		bool	IsDynamic	= true;  // True - the other parameters are ignored and set dynamically at render time
		int		OffsetX		= 0;
		int		OffsetY		= 0;
		uint32	Width		= 0;
		uint32	Height		= 0;
	};

	struct RasterizationDesc
	{
		bool			DepthClampEnable		= false;
		bool			DiscardEnable			= false;
		EPolygonMode	PolygonMode				= EPolygonMode::NONE;
		float			LineWidth				= 1.0f;
		ECullMode		CullMode				= ECullMode::NONE;
		bool			ClockwiseFrontFace		= false;
		bool			DepthBiasEnable			= false;
		float			DepthBiasFactor			= 0.0f;
		float			DepthBiasClamp			= 0.0f;
		float			DepthBiasSlopeFactor	= 0.0f;
	};

	struct StencilOpStateDesc
	{
		EStencilOp		FailOp		= EStencilOp::KEEP;
		EStencilOp		PassOp		= EStencilOp::KEEP;
		EStencilOp		DepthFailOp	= EStencilOp::KEEP;
		ECompareOp		CompareOp	= ECompareOp::NEVER;
		uint32			CompareMask	= 0;
		uint32			WriteMask	= 0;
		uint32			Reference	= 0;
	};

	struct DepthStencilDesc
	{
		bool				DepthTestEnable			= false;
		bool				DepthWriteEnable		= false;
		ECompareOp			DepthCompareOp			= ECompareOp::NEVER;
		bool				DepthBoundsTestEnable	= false;
		bool				StencilTestEnable		= false;
		StencilOpStateDesc	Front					= {};
		StencilOpStateDesc	Back					= {};
		float				MinDepthBounds			= 0.0f;
		float				MaxDepthBounds			= 0.0f;
	};

	struct ColorBlendAttachmentDesc
	{
		bool				BlendEnable			= false;
		EBlendFactor		SrcColorBlendFactor	= EBlendFactor::ZERO;
		EBlendFactor		DstColorBlendFactor	= EBlendFactor::ZERO;
		EBlendOp			ColorBlendOp		= EBlendOp::NONE;
		EBlendFactor		SrcAlphaBlendFactor	= EBlendFactor::ZERO;
		EBlendFactor		DstAlphaBlendFactor	= EBlendFactor::ZERO;
		EBlendOp			AlphaBlendOp		= EBlendOp::NONE;
		FColorComponentFlag	ColorWriteMask		= FColorComponentFlag::NONE;
	};

	struct ColorBlendStateDesc
	{
		std::vector<ColorBlendAttachmentDesc>	ColorBlendAttachments;
		bool									LogicOpEnable	= false;
		ELogicOp								LogicOp			= ELogicOp::NONE;
		float									BlendConstants[4];
	};

	struct GraphicsPipelineDesc
	{
		std::vector<VertexInput> VertexInputs;
		InputAssemblyDesc	InputAssembly	= {};
		ViewportDesc		Viewport		= {};
		ScissorDesc			Scissor			= {};
		RasterizationDesc	Rasterization	= {};
		// Multisampling is not currently added for pipelines
		DepthStencilDesc	DepthStencil	= {};
		ColorBlendStateDesc	ColorBlendState	= {};

		PipelineLayout*		pPipelineLayout	= nullptr;
		GraphicsRenderPass*	pRenderPass		= nullptr;

		uint32				Subpass			= 0;

		// Shaders
		Shader*				pVertexShader	= nullptr;
		Shader*				pFragmentShader	= nullptr;
	};

	class GraphicsPipeline : public Pipeline
	{
	public:
		CLASS_ABSTRACT(GraphicsPipeline);

		/**
		 * Init the Buffer object
		 * @param desc	Buffer creation description
		*/
		virtual void Init(const GraphicsPipelineDesc* pDesc) = 0;

		/**
		 * @return Pipeline layout for this pipeline
		 */
		virtual PipelineLayout* GetPipelineLayout() const = 0;

		/**
		 * @return Native handle to the API specific object
		 */
		virtual uint64 GetNative() const = 0;

	protected:
		GraphicsPipelineDesc	p_PipelineDesc;
	};
}