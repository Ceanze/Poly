#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace Poly
{
	struct TextureTransitionPlan
	{
		std::string    ResolvedName;
		ETextureLayout OldLayout  = ETextureLayout::UNDEFINED;
		ETextureLayout NewLayout  = ETextureLayout::UNDEFINED;
		FAccessFlag    SrcAccess  = FAccessFlag::NONE;
		FAccessFlag    DstAccess  = FAccessFlag::NONE;
		FPipelineStage SrcStage   = FPipelineStage::NONE;
		FPipelineStage DstStage   = FPipelineStage::NONE;
		FImageViewFlag AspectMask = FImageViewFlag::NONE;
	};

	struct BufferTransitionPlan
	{
		std::string    ResolvedName;
		FAccessFlag    SrcAccess = FAccessFlag::NONE;
		FAccessFlag    DstAccess = FAccessFlag::NONE;
		FPipelineStage SrcStage  = FPipelineStage::NONE;
		FPipelineStage DstStage  = FPipelineStage::NONE;
	};

	// A batch of same-queue transitions meant to be issued as a single CommandBuffer::PipelineBarrier call.
	struct BarrierGroup
	{
		std::vector<TextureTransitionPlan> Textures;
		std::vector<BufferTransitionPlan>  Buffers;
	};

	// One resource's half of a cross-queue ownership transfer - Release runs on the source queue,
	// Acquire runs on the destination queue, paired via a SyncPoint wait/signal.
	struct QueueReleasePlan
	{
		std::string    ResolvedName;
		bool           IsTexture = false;
		ETextureLayout OldLayout = ETextureLayout::UNDEFINED;
		ETextureLayout NewLayout = ETextureLayout::UNDEFINED;
		FAccessFlag    SrcAccess = FAccessFlag::NONE;
		FPipelineStage SrcStage  = FPipelineStage::NONE;
		FQueueType     DstQueue  = FQueueType::NONE;
	};

	struct QueueAcquirePlan
	{
		std::string    ResolvedName;
		bool           IsTexture = false;
		ETextureLayout OldLayout = ETextureLayout::UNDEFINED;
		ETextureLayout NewLayout = ETextureLayout::UNDEFINED;
		FAccessFlag    DstAccess = FAccessFlag::NONE;
		FPipelineStage DstStage  = FPipelineStage::NONE;
		FQueueType     SrcQueue  = FQueueType::NONE;
	};

	struct PassSyncPlan
	{
		size_t                        PassIndex = 0;
		BarrierGroup                  PreBarriers;              // same-queue transitions, batched into one call
		std::vector<QueueAcquirePlan> Acquires;                 // cross-queue acquires needed before this pass runs
		std::vector<QueueReleasePlan> PostReleases;             // cross-queue releases to run right after this pass -
		                                                        // attached here because this pass was the resource's
		                                                        // last user, even though the consumer needing the
		                                                        // transfer is later in the schedule
		BarrierGroup PostBarriers;                              // same-queue transitions to run right after this pass,
		                                                        // for resources with a declared WithFinalState()
		std::unordered_map<FQueueType, uint64_t> RequiredWaits; // per source-queue (in QueueAcquirePlan), minimum SyncPoint value to wait
		                                                        // for before this pass runs; absent = no wait needed
		uint64_t SubmissionIndex = 0;                           // 1-based position within this pass's own queue's submission order; doubles as
		                                                        // the SyncPoint signal value once this pass's work is submitted

		// Load op for each of this pass's attachment writes ($Color/$Depth/$Stencil), keyed by resolved
		// name. Computed once at compile time since it depends only on program order: CLEAR on a
		// resource's first write in the program, LOAD on subsequent writes, unless a pass declared an
		// explicit override (see ResolvedPort::LoadOpOverride).
		std::unordered_map<std::string, ELoadOp> AttachmentLoadOps;
	};

	// The compile-time synchronization plan for a RenderProgram: one PassSyncPlan per pass, in the same
	// order as RenderProgram::GetPasses(). Pure data - a future RenderProgramInstance::Execute() consumes
	// this to issue the actual barriers/acquires/releases/waits.
	class SyncPlan
	{
	public:
		explicit SyncPlan(std::vector<PassSyncPlan> plans);

		const std::vector<PassSyncPlan>& GetPassPlans() const { return m_Plans; }

	private:
		std::vector<PassSyncPlan> m_Plans;
	};
} // namespace Poly
