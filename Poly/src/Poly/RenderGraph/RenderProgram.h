#pragma once

#include "Platform/API/GraphicsPipeline.h"
#include "Poly/RenderGraph/Resource/ResourceState.h"
#include "Poly/RenderGraph/Resource/ResourceType.h"
#include "Poly/RenderGraph/SyncPlan.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

#include <functional>
#include <string>
#include <vector>

namespace Poly
{
	class ExecuteContext;

	struct ResolvedPort
	{
		std::string ShaderName;   // local name in the pass shader
		std::string ResolvedName; // globally resolved name (e.g. "$Color", "SceneAlbedo")
		bool        IsWrite;      // true = output, false = input

		// True if ResolvedName matched a resource registered on the RenderGraph (an externally
		// owned resource supplied per-frame via RenderProgramInstance::UpdateResource()). False
		// means the resource is transient and owned/allocated by the RenderProgramInstance.
		bool IsExternal = false;

		EResourceType  ResourceType = EResourceType::None;
		FResourceState InitialState = FResourceState::Unknown;

		// Overrides the auto-inferred attachment load op ($Color/$Depth/$Stencil only). ELoadOp::NONE
		// means "auto": RenderProgramBuilder::PlanSynchronization clears on a resource's first write
		// in the program and loads on subsequent writes.
		ELoadOp LoadOpOverride = ELoadOp::NONE;

		// Explicit size from the matching IResourceDeclaration::WithSize(), or 0 if unset - in
		// which case the resource is sized to the render target (see RenderProgramInstance).
		uint32 Width  = 0;
		uint32 Height = 0;
	};

	// Used to resolve bindless slot
	struct ResolvedSlot
	{
		uint32      Slot = 0;
		std::string ResourceName;
	};

	struct ResolvedPass
	{
		std::string               Name;
		std::vector<ResolvedPort> Ports;

		std::vector<std::pair<std::string, FShaderStage>> Shaders;
		GraphicsPipelineDesc                              PipelineDesc;
		std::function<void(ExecuteContext&)>              ExecuteFn;

		FQueueType Queue = FQueueType::GRAPHICS;

		std::vector<ResolvedSlot> BufferSlots;
		std::vector<ResolvedSlot> TextureSlots;
		uint32                    BufferSlotsOffset  = 0;
		uint32                    TextureSlotsOffset = 0;
		uint32                    PushConstantSize   = 0;
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
