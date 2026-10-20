#pragma once

#include "Platform/API/GraphicsPipeline.h"
#include "Poly/RenderGraph/Feature/FeaturePort.h"
#include "Poly/RenderGraph/Pass/PassDesc.h"
#include "Poly/RenderGraph/Resource/ResourceScope.h"
#include "Poly/RenderGraph/Resource/ResourceState.h"
#include "Poly/RenderGraph/Resource/ResourceType.h"
#include "Poly/RenderGraph/SyncPlan.h"
#include "Poly/RenderGraph/TextureField.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

#include <functional>
#include <string>
#include <vector>

namespace Poly
{
	class ExecuteContext;

	struct ResolvedPort
	{
		// Value of TargetSlot for ports that are not the color target of a view
		static constexpr uint32 NO_TARGET = ~0u;

		std::string ShaderName; // local name in the pass shader

		// Name of the resource before it is scoped (e.g. "$Depth", "scene.vertices"): what external resources are
		// looked up by in a RenderResourceTable and what the transient resources of a view are stored by
		std::string Name;

		// Name identifying the resource within the render program, Name prefixed according to Scope
		// (e.g. "view#1.$Depth", "world#0.scene.vertices", "target#2.$Color")
		std::string ResolvedName;

		bool IsWrite = false; // true = output, false = input

		// Who provides the resource, see EResourceScope. Semantic depth/stencil ports and the imports/exports
		// between the passes of a view stage are view scoped.
		EResourceScope Scope = EResourceScope::Global;

		// The feature port the pass mapped with MapResource(), None for every other port
		EFeaturePort Semantic = EFeaturePort::None;

		// Set when the resource is the color target of a view: the view slot (see ResolvedPass::ViewSlot) whose
		// target it is, 0 being the window. This is every EFeaturePort::Color port, and every read of a view
		// scoped texture that the view has bound to the target of another view.
		uint32 TargetSlot = NO_TARGET;

		// True if Name matched a resource registered in the RenderCatalog without a size: an externally owned
		// resource supplied through a RenderResourceTable. False means the resource is transient and
		// allocated for the render program.
		bool IsExternal = false;

		EResourceType  ResourceType = EResourceType::None;
		FResourceState InitialState = FResourceState::Unknown;

		// Set for ports declared via ReadResource()/WriteResource(): the state the resource must be in for
		// this pass. Such ports have an empty ShaderName (no shader binding). Unknown means the usage is
		// derived from ResourceType instead (DeriveResourceUsage).
		FResourceState UsageState = FResourceState::Unknown;

		// Overrides the auto-inferred attachment load op ($Color/$Depth/$Stencil only). ELoadOp::NONE
		// means "auto": RenderProgramBuilder::PlanSynchronization clears on a resource's first write
		// in the program and loads on subsequent writes.
		ELoadOp LoadOpOverride = ELoadOp::NONE;

		// Explicit size from the matching IResourceDeclaration::WithSize(), or 0 if unset - in
		// which case the resource is sized to the render target (see RenderProgramInstance).
		uint32 Width  = 0;
		uint32 Height = 0;
	};

	// A port bound to a field of the pass's push constants, matched by the shader name of the port
	struct ResolvedBinding
	{
		uint32 Offset    = 0; // byte offset of the field within the push constant block
		uint32 PortIndex = 0; // index into ResolvedPass::Ports of the port bound to the field
	};

	struct ResolvedPass
	{
		std::string               Name;
		std::vector<ResolvedPort> Ports;

		// Index of the RenderViewContext the pass is recorded with: 0 for the passes of frame features,
		// 1 and up for the views of the program, in the order of its ProgramKey
		uint32 ViewSlot = 0;

		std::vector<PassShader>              Shaders;
		GraphicsPipelineDesc                 PipelineDesc;
		std::function<void(ExecuteContext&)> ExecuteFn;

		FQueueType Queue = FQueueType::GRAPHICS;

		std::vector<ResolvedBinding> BufferBindings;
		std::vector<ResolvedBinding> TextureBindings;
		uint32                       PushConstantSize = 0;

		// Every texture field of the push constants, bound to a port or not, for ExecuteContext::SetTexture
		std::vector<TextureField> TextureFields;
	};

	class RenderProgram
	{
	public:
		explicit RenderProgram(std::vector<ResolvedPass> sortedPasses, SyncPlan syncPlan);

		const std::vector<ResolvedPass>& GetPasses() const { return m_Passes; }
		const SyncPlan&                  GetSyncPlan() const { return m_SyncPlan; }

	private:
		std::vector<ResolvedPass> m_Passes;
		SyncPlan                  m_SyncPlan;
	};
} // namespace Poly
