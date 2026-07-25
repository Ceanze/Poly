#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "RenderProgram.h"

#include <array>
#include <mutex>
#include <unordered_map>

namespace Poly
{
	struct RenderView;
	class Buffer;
	class Texture;
	class Sampler;
	class SyncPoint;
	class CommandPool;
	class CommandBuffer;
	class TextureView;
	class PipelineLayout;
	class GraphicsPipeline;

	/*
	 * An instantiated, active version of a RenderProgram. Holds the compiled RenderProgram
	 * together with the runtime graphics resources allocated for it.
	 *
	 * One instance exists per window (see Renderer::WindowContext) so that each window can
	 * render a different view (camera/scene/target) and keep its own frame-in-flight state,
	 * while all instances may share the same compiled RenderProgram.
	 *
	 * Ownership: constructed by Renderer for each window once a RenderProgram becomes active,
	 * at a point where it's safe to retire the previously active instance (see Renderer).
	 */
	class RenderProgramInstance
	{
	public:
		static constexpr uint32 FRAMES_IN_FLIGHT = 2;

		explicit RenderProgramInstance(Ref<RenderProgram> pRenderProgram);
		~RenderProgramInstance() = default;
		CLASS_REMOVE_COPY(RenderProgramInstance);

		void Execute(const RenderView& view);

		// Supplies (or replaces) an externally-owned resource for a port whose ResolvedPort::IsExternal
		// is true - i.e. anything not registered with an explicit size on the RenderGraph. Looked up by
		// ResolvedPort::ResolvedName ("passName#N.resName" for a feature-scoped import/export, or a bare
		// global/semantic name). Safe to call once at setup for a resource that never changes, or every
		// frame for one that does (e.g. re-pointing at a new frame's data). A buffer supplied here that
		// will be accessed via BDA in a shader must have been created with FBufferUsage::SHADER_DEVICE_ADDRESS.
		void UpdateResource(std::string_view resolvedName, Ref<Buffer> pBuffer);
		void UpdateResource(std::string_view resolvedName, Ref<TextureView> pTextureView, Ref<Sampler> pSampler = nullptr);

		const RenderProgram& GetProgram() const { return *m_pRenderProgram; }

	private:
		// A resolved-name's backing GPU resource - either supplied externally via UpdateResource(), or
		// allocated internally on first touch (texture-shaped only - see RenderProgramInstance.cpp for
		// why graph-owned buffers aren't reachable yet). Exactly one of pBuffer/pTextureView is set.
		struct RuntimeResource
		{
			Ref<Buffer>      pBuffer;
			Ref<Texture>     pTexture; // only set for internally-allocated textures - keeps the Texture alive alongside pTextureView
			Ref<TextureView> pTextureView;
			Ref<Sampler>     pSampler;

			// Cached bindless heap registration, assigned once the first time this resource is used
			// in a bindless slot (see BindlessManager) - avoids re-registering the same resource every frame.
			uint32 BindlessHeapIndex = ~0u;

			bool IsBuffer() const { return pBuffer != nullptr; }
			bool IsTexture() const { return pTextureView != nullptr; }
		};

		// Per-pass runtime state, stable for the RenderProgramInstance's lifetime (ResolvedPass list
		// never changes) except for the command buffers, which are re-recorded every frame.
		struct PerPassResources
		{
			std::array<Ref<CommandPool>, FRAMES_IN_FLIGHT> CommandPools;
			std::array<CommandBuffer*, FRAMES_IN_FLIGHT>   CommandBuffers{};
			Ref<PipelineLayout>                            Layout;
			Ref<GraphicsPipeline>                          Pipeline;
		};

		void EnsurePerPassResources();
		void WaitForFrameSlotReuse(uint32 frameIndex);

		CommandBuffer*    GetCommandBuffer(size_t passIndex) const { return m_PassResources[passIndex].CommandBuffers[m_FrameIndex]; }
		PipelineLayout*   GetOrCreatePipelineLayout(size_t passIndex);
		GraphicsPipeline* GetOrCreatePipeline(size_t passIndex, const RenderView& view);

		RuntimeResource* ResolvePort(const ResolvedPort& port, const RenderView& view);
		EFormat          GetPortFormat(const ResolvedPort& port, const RenderView& view);
		uint32           GetOrRegisterBindlessTextureIndex(RuntimeResource* pResource, ETextureLayout layout);

		Texture* GetTextureForBarrier(const std::string& resolvedName, const RenderView& view);
		Buffer*  GetBufferForBarrier(const std::string& resolvedName);

		void RecordPass(size_t passIndex, const RenderView& view);
		void BuildPushConstants(size_t passIndex, std::vector<byte>& outData);
		void ApplyAcquire(CommandBuffer* pCmd, const struct QueueAcquirePlan& acquire, FQueueType currentQueue, const RenderView& view);
		void ApplyRelease(CommandBuffer* pCmd, const struct QueueReleasePlan& release, FQueueType currentQueue, const RenderView& view);
		void ApplyBarrierGroup(CommandBuffer* pCmd, const struct BarrierGroup& group, const RenderView& view);

		SyncPoint* GetOrCreateQueueSyncPoint(FQueueType queue);

		Ref<RenderProgram> m_pRenderProgram;
		uint32             m_FrameIndex  = 0;
		bool               m_Initialized = false;

		std::vector<PerPassResources> m_PassResources; // indexed by pass index

		// Guards m_Resources (and each entry's BindlessHeapIndex) - RecordPass runs concurrently across
		// passes, and two passes can reference the same resolved resource name (e.g. a shared global).
		// Recursive because ResolvePort/BuildPushConstants/GetOrRegisterBindlessTextureIndex call into
		// each other while already holding the lock.
		std::recursive_mutex                             m_ResourcesMutex;
		std::unordered_map<std::string, RuntimeResource> m_Resources; // keyed by ResolvedPort::ResolvedName

		std::unordered_map<FQueueType, Ref<SyncPoint>> m_QueueSyncPoints;
		std::unordered_map<FQueueType, uint64>         m_QueueTimelineBase;

		// Highest signal value each queue reached the last time this frame-in-flight slot was used -
		// waited on before that slot's command pools are reset & reused again.
		std::array<std::unordered_map<FQueueType, uint64>, FRAMES_IN_FLIGHT> m_FrameReclaimValues;
	};
} // namespace Poly
