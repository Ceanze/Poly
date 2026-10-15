#pragma once

#include "BlendMode.h"
#include "Platform/API/GraphicsPipeline.h"
#include "Poly/RenderGraph/Feature/FeaturePort.h"
#include "Poly/RenderGraph/Resource/ResourceState.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

#include <optional>
#include <string>
#include <vector>

namespace Poly
{
	// One stage of a .slang shader used by a pass, see IPassDeclaration::WithShader
	struct PassShader
	{
		std::string  Path;
		FShaderStage Stage = FShaderStage::NONE;
	};

	// A mapped feature port, plus an optional attachment load-op override. LoadOpOverride left at
	// ELoadOp::NONE means "auto" - RenderProgramBuilder infers CLEAR on a resource's first write in
	// the program and LOAD on subsequent writes (see PlanSynchronization).
	struct ResourceMapping
	{
		EFeaturePort Port = EFeaturePort::None;
		std::string  ShaderResourceName;
		ELoadOp      LoadOpOverride = ELoadOp::NONE;
	};

	// A named resource bound to a shader variable of a pass, see IPassDeclaration::MapGlobal/ImportResource/ExportResource
	struct ResourceBinding
	{
		std::string ResourceName;
		std::string ShaderResourceName;
	};

	// A resource used by a pass without a shader binding, see IPassDeclaration::ReadResource/WriteResource.
	struct ResourceUse
	{
		std::string    Name;
		FResourceState State   = FResourceState::Unknown;
		bool           IsWrite = false;
	};

	struct PassVertexAttribute
	{
		uint32  Location = 0;
		EFormat Format   = EFormat::UNDEFINED;
		uint32  Offset   = 0;
	};

	// The vertex buffer a pass binds, see IPassDeclaration::WithVertexLayout/WithVertexAttribute
	struct PassVertexLayout
	{
		uint32                           Stride    = 0;
		EVertexInputRate                 InputRate = EVertexInputRate::VERTEX;
		std::vector<PassVertexAttribute> Attributes;
	};

	struct PassDepthBias
	{
		float ConstantFactor = 0.0f;
		float SlopeFactor    = 0.0f;
		float Clamp          = 0.0f;
	};

	struct PassStencilState
	{
		StencilOpStateDesc Front = {};
		StencilOpStateDesc Back  = {};
	};

	/*
	 * The pipeline state a pass sets explicitly. A field left unset keeps its default, or what follows from
	 * the mapped feature ports (depth test/write), see PassDeclaration::BuildPipelineDesc().
	 */
	struct PassPipelineState
	{
		std::optional<ETopology>    Topology;
		std::optional<EPolygonMode> PolygonMode;
		std::optional<ECullMode>    CullMode;
		std::optional<bool>         FrontFaceClockwise;

		std::optional<bool>          DepthTest;
		std::optional<bool>          DepthWrite;
		std::optional<ECompareOp>    DepthCompareOp;
		std::optional<PassDepthBias> DepthBias;

		// Setting it enables the stencil test
		std::optional<PassStencilState> Stencil;

		// Blend state of every color output of the pass. CustomBlend is used over BlendMode if both are set
		std::optional<EBlendMode>               BlendMode;
		std::optional<ColorBlendAttachmentDesc> CustomBlend;

		// Setting it enables the logic op
		std::optional<ELogicOp> LogicOp;

		// Static viewport/scissor, dynamic if unset
		std::optional<ViewportDesc> Viewport;
		std::optional<ScissorDesc>  Scissor;
	};

	/*
	 * Everything about a pass that is data: what PassDeclaration builds up and what is written to a pass file.
	 * The functions of a pass (execute, setup, pipeline override) are not part of it.
	 */
	struct PassDesc
	{
		std::string Name;
		FQueueType  Queue = FQueueType::GRAPHICS;

		std::vector<PassShader> Shaders;

		std::vector<ResourceMapping> ResourceMappings;
		std::vector<ResourceBinding> GlobalMappings;
		std::vector<ResourceBinding> ImportedResources;
		std::vector<ResourceBinding> ExportedResources;
		std::vector<ResourceUse>     ResourceUses;

		PassVertexLayout  VertexLayout;
		PassPipelineState Pipeline;
	};
} // namespace Poly
