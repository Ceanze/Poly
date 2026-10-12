#pragma once

#include "PipelineCache.h"
#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "RenderProgram.h"
#include "ResourceManager.h"

#include <array>
#include <span>
#include <unordered_map>

namespace Poly
{
	struct RenderViewContext;
	class RenderResourceTable;
	class Buffer;
	class Texture;
	class Sampler;
	class SyncPoint;
	class CommandPool;
	class CommandBuffer;
	class BinarySemaphore;
	class TextureView;
	class PipelineLayout;
	class GraphicsPipeline;

	/*
	 * An instantiated, active version of a RenderProgram: the compiled program together with what is needed
	 * to record and submit it, i.e. the command buffers of its passes and its frame-in-flight state.
	 *
	 * Everything that has to outlive a switch of the active program is owned elsewhere and handed in: pipelines
	 * by the PipelineCache, the resources of a view by the view (see RenderViewContext::pTransients).
	 *
	 * Ownership: constructed by the Renderer the first time a program is needed and kept with it in the
	 * renderer's program cache. Destroying an instance waits for the GPU to finish the frames it submitted.
	 */
	class RenderProgramInstance
	{
	public:
		static constexpr uint32 FRAMES_IN_FLIGHT = 2;

		/**
		 * @param pRenderProgram - program to execute
		 * @param pipelineCache - cache to get the pipelines of the passes from
		 */
		RenderProgramInstance(Ref<RenderProgram> pRenderProgram, PipelineCache& pipelineCache);
		~RenderProgramInstance();
		CLASS_REMOVE_COPY(RenderProgramInstance);

		/**
		 * Records and submits all passes of the program
		 * @param views - one context per view slot of the program: the frame itself at index 0, followed by the
		 *                views in the order of the ProgramKey the program was built from
		 * @param pAcquireSemaphore - semaphore of the swapchain image being rendered to
		 */
		void Execute(std::span<const RenderViewContext> views, BinarySemaphore* pAcquireSemaphore = nullptr);

		const RenderProgram& GetProgram() const { return *m_pRenderProgram; }

	private:
		// The GPU resource backing a resolved name for the frame being executed
		struct RuntimeResource
		{
			BufferHandle  BufHandle;
			TextureHandle TexHandle; // invalid for the backbuffer of the window, which has no handle
			SamplerHandle SamplerHnd;
			Texture*      pTexture     = nullptr;
			TextureView*  pTextureView = nullptr;

			bool IsBuffer() const { return BufHandle.IsValid(); }
			bool IsTexture() const { return pTexture != nullptr; }
		};

		// Per-pass runtime state, stable for the RenderProgramInstance's lifetime (ResolvedPass list
		// never changes) except for the command buffers, which are re-recorded every frame.
		struct PerPassResources
		{
			std::array<Ref<CommandPool>, FRAMES_IN_FLIGHT> CommandPools;
			std::array<CommandBuffer*, FRAMES_IN_FLIGHT>   CommandBuffers{};

			PipelineCache::Entry Pipeline;
		};

		void EnsurePerPassResources();
		void WaitForFrameSlotReuse(uint32 frameIndex);

		// Finds the resource behind every port for this frame and the pipeline of every pass. Done up front
		// and in program order, so that recording the passes in parallel only reads.
		void ResolveResources(std::span<const RenderViewContext> views);
		bool ResolveExternal(const ResolvedPort& port, const RenderViewContext& view, RuntimeResource& outResource) const;
		bool ResolveTransient(const ResolvedPass& pass, const ResolvedPort& port, std::span<const RenderViewContext> views, RuntimeResource& outResource) const;
		void ResolvePipelines();

		const RuntimeResource* FindResource(const std::string& resolvedName) const;

		CommandBuffer* GetCommandBuffer(size_t passIndex) const { return m_PassResources[passIndex].CommandBuffers[m_FrameIndex]; }
		uint32         GetBindlessIndex(const RuntimeResource& resource) const;

		void RecordPass(size_t passIndex, const RenderViewContext& view);
		void BuildPushConstants(size_t passIndex, std::vector<byte>& outData) const;
		void ApplyAcquire(CommandBuffer* pCmd, const struct QueueAcquirePlan& acquire, FQueueType currentQueue) const;
		void ApplyRelease(CommandBuffer* pCmd, const struct QueueReleasePlan& release, FQueueType currentQueue) const;
		void ApplyBarrierGroup(CommandBuffer* pCmd, const struct BarrierGroup& group) const;

		SyncPoint* GetOrCreateQueueSyncPoint(FQueueType queue);

		Ref<RenderProgram> m_pRenderProgram;
		PipelineCache&     m_PipelineCache;
		uint32             m_FrameIndex  = 0;
		bool               m_Initialized = false;

		std::vector<PerPassResources> m_PassResources; // indexed by pass index

		// Rebuilt every frame by ResolveResources(), keyed by ResolvedPort::ResolvedName
		std::unordered_map<std::string, RuntimeResource> m_Resources;

		std::unordered_map<FQueueType, Ref<SyncPoint>> m_QueueSyncPoints;
		std::unordered_map<FQueueType, uint64>         m_QueueTimelineBase;

		// Highest signal value each queue reached the last time this frame-in-flight slot was used -
		// waited on before that slot's command pools are reset & reused again.
		std::array<std::unordered_map<FQueueType, uint64>, FRAMES_IN_FLIGHT> m_FrameReclaimValues;
	};
} // namespace Poly
