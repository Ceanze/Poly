#include "RenderProgramInstance.h"

#include "ExecuteContext.h"
#include "Platform/API/BinarySemaphore.h"
#include "Platform/API/Buffer.h"
#include "Platform/API/CommandBuffer.h"
#include "Platform/API/CommandPool.h"
#include "Platform/API/CommandQueue.h"
#include "Platform/API/DescriptorSet.h"
#include "Platform/API/GraphicsPipeline.h"
#include "Platform/API/PipelineLayout.h"
#include "Platform/API/Sampler.h"
#include "Platform/API/SyncPoint.h"
#include "Platform/API/Texture.h"
#include "Platform/API/TextureView.h"
#include "Poly/Core/RenderAPI.h"
#include "Poly/Core/ThreadPool.h"
#include "Poly/World/World.h"
#include "RenderResourceTable.h"
#include "RenderTransientResources.h"
#include "RenderViewContext.h"
#include "Resource/ResourceUsage.h"

#include <cstring>

namespace Poly
{
	RenderProgramInstance::RenderProgramInstance(Ref<RenderProgram> pRenderProgram, PipelineCache& pipelineCache)
	    : m_pRenderProgram(std::move(pRenderProgram))
	    , m_PipelineCache(pipelineCache)
	{}

	RenderProgramInstance::~RenderProgramInstance()
	{
		for (uint32 frameIndex = 0; frameIndex < FRAMES_IN_FLIGHT; frameIndex++)
			WaitForFrameSlotReuse(frameIndex);
	}

	void RenderProgramInstance::Execute(std::span<const RenderViewContext> views, BinarySemaphore* pAcquireSemaphore)
	{
		if (!m_Initialized)
		{
			EnsurePerPassResources();
			m_Initialized = true;
		}

		WaitForFrameSlotReuse(m_FrameIndex);
		ResolveResources(views);
		ResolvePipelines();

		const auto& passes    = m_pRenderProgram->GetPasses();
		const auto& passPlans = m_pRenderProgram->GetSyncPlan().GetPassPlans();

		// Phase A: parallel recording
		std::vector<std::function<void()>> tasks;
		tasks.reserve(passes.size());
		for (size_t i = 0; i < passes.size(); i++)
			tasks.push_back([this, i, &view = views[passes[i].ViewSlot]]() { RecordPass(i, view); });

		ThreadPool::SubmitAndWait(tasks);

		// Phase B: sequential per-queue submit, in program order. Vulkan requires submission order to a
		// queue to match what SyncPlan assumed (same-queue barriers rely on prior work already being
		// enqueued; SubmissionIndex is defined as position within a queue's submission order) - a plain
		// sequential pass over the program-ordered list gives every queue its submissions in order for free.
		std::unordered_map<FQueueType, uint64> highestSubmissionIndexThisFrame;
		for (size_t i = 0; i < passes.size(); i++)
		{
			const ResolvedPass& pass            = passes[i];
			const PassSyncPlan& plan            = passPlans[i];
			SyncPoint*          pQueueSyncPoint = GetOrCreateQueueSyncPoint(pass.Queue);
			const uint64        base            = m_QueueTimelineBase[pass.Queue];

			SubmitDesc submitDesc     = {};
			submitDesc.CommandBuffers = {GetCommandBuffer(i)};
			if (i == 0 && pAcquireSemaphore)
				submitDesc.WaitSemaphores = {pAcquireSemaphore};

			for (const auto& [srcQueue, waitValue] : plan.RequiredWaits)
			{
				SyncPoint* pSrcSyncPoint = GetOrCreateQueueSyncPoint(srcQueue);
				submitDesc.WaitSyncPoints.push_back({pSrcSyncPoint, m_QueueTimelineBase[srcQueue] + waitValue});
			}

			// Acquire any pending uploads for each port resource, handles upload sync and queue acqusition.
			for (const ResolvedPort& port : pass.Ports)
			{
				const RuntimeResource* pRes = FindResource(port.ResolvedName);
				if (!pRes || (!pRes->TexHandle.IsValid() && !pRes->BufHandle.IsValid()))
					continue;

				SyncPoint* pUploadSyncPoint = nullptr;
				uint64     uploadValue      = 0;
				const bool hasPendingUpload = pRes->TexHandle.IsValid() ? ResourceManager::ConsumePendingUploadSync(pRes->TexHandle, &pUploadSyncPoint, &uploadValue)
				                                                        : ResourceManager::ConsumePendingUploadSync(pRes->BufHandle, &pUploadSyncPoint, &uploadValue);
				if (hasPendingUpload)
					submitDesc.WaitSyncPoints.push_back({pUploadSyncPoint, uploadValue});
			}

			const uint64 signalValue    = base + plan.SubmissionIndex;
			submitDesc.SignalSyncPoints = {{pQueueSyncPoint, signalValue}};

			RenderAPI::GetCommandQueue(pass.Queue)->Submit(submitDesc);

			uint64& highest                                = highestSubmissionIndexThisFrame[pass.Queue];
			highest                                        = std::max(highest, plan.SubmissionIndex);
			m_FrameReclaimValues[m_FrameIndex][pass.Queue] = signalValue;
		}

		for (const auto& [queue, count] : highestSubmissionIndexThisFrame)
			m_QueueTimelineBase[queue] += count;

		m_FrameIndex = (m_FrameIndex + 1) % FRAMES_IN_FLIGHT;
	}

	void RenderProgramInstance::EnsurePerPassResources()
	{
		const auto& passes = m_pRenderProgram->GetPasses();
		m_PassResources.resize(passes.size());

		for (size_t i = 0; i < passes.size(); i++)
		{
			PerPassResources& res = m_PassResources[i];
			for (uint32 f = 0; f < FRAMES_IN_FLIGHT; f++)
			{
				res.CommandPools[f]   = RenderAPI::CreateCommandPool(passes[i].Queue, FCommandPoolFlags::RESET_COMMAND_BUFFERS);
				res.CommandBuffers[f] = res.CommandPools[f]->AllocateCommandBuffer(ECommandBufferLevel::PRIMARY);
			}
		}
	}

	void RenderProgramInstance::WaitForFrameSlotReuse(uint32 frameIndex)
	{
		for (const auto& [queue, value] : m_FrameReclaimValues[frameIndex])
			GetOrCreateQueueSyncPoint(queue)->Wait(value);
	}

	void RenderProgramInstance::ResolveResources(std::span<const RenderViewContext> views)
	{
		m_Resources.clear();

		for (const ResolvedPass& pass : m_pRenderProgram->GetPasses())
		{
			const RenderViewContext& view = views[pass.ViewSlot];

			for (const ResolvedPort& port : pass.Ports)
			{
				if (m_Resources.contains(port.ResolvedName))
					continue;

				RuntimeResource resource;
				bool            resolved = false;

				if (port.TargetSlot != ResolvedPort::NO_TARGET)
				{
					// The color target of a view, either written by the pass or read from an earlier view
					const RenderViewContext& targetView = views[port.TargetSlot];
					if (targetView.pTarget)
					{
						resource.TexHandle    = targetView.TargetHandle;
						resource.SamplerHnd   = ResourceManager::GetDefaultLinearSampler();
						resource.pTextureView = targetView.pTarget;
						resource.pTexture     = targetView.pTarget->GetTexture();
						resolved              = true;
					}
				}
				else if (port.IsExternal)
				{
					resolved = ResolveExternal(port, view, resource);
				}
				else
				{
					resolved = ResolveTransient(pass, port, views, resource);
				}

				if (resolved)
					m_Resources.emplace(port.ResolvedName, resource);
			}
		}
	}

	// The scope of a resource decides which table it is provided through
	bool RenderProgramInstance::ResolveExternal(const ResolvedPort& port, const RenderViewContext& view, RuntimeResource& outResource) const
	{
		const RenderResourceTable* pTable = nullptr;
		switch (port.Scope)
		{
		case EResourceScope::Global:
			pTable = view.pGlobalResources;
			break;
		case EResourceScope::World:
			pTable = view.pWorld ? &view.pWorld->GetRenderResources() : nullptr;
			break;
		case EResourceScope::View:
			pTable = view.pViewResources;
			break;
		}

		const RenderResourceTable::Entry* pEntry = nullptr;
		if (pTable)
		{
			auto it = pTable->GetEntries().find(port.Name);
			if (it != pTable->GetEntries().end() && (it->second.BufHandle.IsValid() || it->second.TexHandle.IsValid()))
				pEntry = &it->second;
		}

		if (!pEntry)
		{
			POLY_CORE_WARN("Resource '{}' is not provided by the {} resource table", port.Name,
			               port.Scope == EResourceScope::Global ? "global" : (port.Scope == EResourceScope::World ? "world" : "view"));
			return false;
		}

		outResource.BufHandle = pEntry->BufHandle;
		outResource.TexHandle = pEntry->TexHandle;
		if (pEntry->TexHandle.IsValid())
		{
			outResource.SamplerHnd   = pEntry->SamplerHnd.IsValid() ? pEntry->SamplerHnd : ResourceManager::GetDefaultLinearSampler();
			outResource.pTexture     = ResourceManager::Resolve(pEntry->TexHandle);
			outResource.pTextureView = ResourceManager::ResolveView(pEntry->TexHandle);
		}

		return outResource.IsBuffer() || outResource.IsTexture();
	}

	// Graph-owned texture, sized either explicitly (WithSize()) or to the render target of the view
	bool RenderProgramInstance::ResolveTransient(const ResolvedPass& pass, const ResolvedPort& port, std::span<const RenderViewContext> views, RuntimeResource& outResource) const
	{
		const bool isDepthSemantic = port.Semantic == EFeaturePort::Depth || port.Semantic == EFeaturePort::Stencil;
		if (!isDepthSemantic && !IsTextureResourceType(port.ResourceType))
		{
			POLY_CORE_ERROR("Resource '{}' is buffer-shaped but not external; graph-owned buffers aren't "
			                "supported yet - provide it through a RenderResourceTable instead.",
			                port.ResolvedName);
			return false;
		}

		const RenderViewContext& view    = views[pass.ViewSlot];
		const Texture*           pTarget = view.pTarget ? view.pTarget->GetTexture() : nullptr;
		const uint32             width   = port.Width != 0 ? port.Width : (pTarget ? pTarget->GetWidth() : 0);
		const uint32             height  = port.Height != 0 ? port.Height : (pTarget ? pTarget->GetHeight() : 0);
		if (width == 0 || height == 0)
			return false;

		// TODO: IResourceDeclaration has no format setter yet (only WithSize/WithType/WithInitialState) -
		// default until it does; only affects graph-owned internal resources, not externally-supplied ones.
		const EFormat       format = port.Semantic == EFeaturePort::Depth     ? EFormat::DEPTH
		                             : port.Semantic == EFeaturePort::Stencil ? EFormat::DEPTH_STENCIL
		                                                                      : EFormat::R8G8B8A8_UNORM;
		const FTextureUsage usage  = isDepthSemantic ? FTextureUsage::DEPTH_STENCIL_ATTACHMENT | FTextureUsage::SAMPLED
		                                             : FTextureUsage::SAMPLED | (port.ResourceType == EResourceType::StorageImage
		                                                                             ? FTextureUsage::STORAGE
		                                                                             : FTextureUsage::COLOR_ATTACHMENT);

		// What belongs to a view is owned by the view and stored by its unscoped name, so that it stays with the
		// view whichever slot the view has in the program. Everything else is owned by the frame.
		const bool                isViewOwned = pass.ViewSlot != 0 && port.Scope == EResourceScope::View;
		RenderTransientResources* pOwner      = isViewOwned ? view.pTransients : views[0].pTransients;
		if (!pOwner)
			return false;

		outResource.TexHandle    = pOwner->GetOrCreate(isViewOwned ? port.Name : port.ResolvedName, width, height, format, usage, port.ResolvedName);
		outResource.SamplerHnd   = ResourceManager::GetDefaultLinearSampler();
		outResource.pTexture     = ResourceManager::Resolve(outResource.TexHandle);
		outResource.pTextureView = ResourceManager::ResolveView(outResource.TexHandle);
		return outResource.IsTexture();
	}

	void RenderProgramInstance::ResolvePipelines()
	{
		const auto& passes = m_pRenderProgram->GetPasses();
		for (size_t i = 0; i < passes.size(); i++)
		{
			std::vector<EFormat> colorFormats;
			EFormat              depthFormat   = EFormat::UNDEFINED;
			EFormat              stencilFormat = EFormat::UNDEFINED;
			bool                 isComplete    = true;

			for (const ResolvedPort& port : passes[i].Ports)
			{
				if (!port.IsWrite || !IsAttachmentSemanticPort(port.Semantic))
					continue;

				const RuntimeResource* pRes = FindResource(port.ResolvedName);
				if (!pRes || !pRes->IsTexture())
				{
					isComplete = false;
					break;
				}

				const EFormat format = pRes->pTexture->GetDesc().Format;
				if (port.Semantic == EFeaturePort::Color)
					colorFormats.push_back(format);
				else if (port.Semantic == EFeaturePort::Depth)
					depthFormat = format;
				else
					stencilFormat = format;
			}

			// A pass missing one of its attachments is not rendered this frame, see RecordPass()
			m_PassResources[i].Pipeline = isComplete ? m_PipelineCache.GetOrCreate(passes[i], colorFormats, depthFormat, stencilFormat) : PipelineCache::Entry{};
		}
	}

	const RenderProgramInstance::RuntimeResource* RenderProgramInstance::FindResource(const std::string& resolvedName) const
	{
		auto it = m_Resources.find(resolvedName);
		return it != m_Resources.end() ? &it->second : nullptr;
	}

	SyncPoint* RenderProgramInstance::GetOrCreateQueueSyncPoint(FQueueType queue)
	{
		auto it = m_QueueSyncPoints.find(queue);
		if (it != m_QueueSyncPoints.end())
			return it->second.get();

		Ref<SyncPoint> pSyncPoint = RenderAPI::CreateSyncPoint();
		auto [insertedIt, _]      = m_QueueSyncPoints.emplace(queue, std::move(pSyncPoint));
		m_QueueTimelineBase.emplace(queue, 0);
		return insertedIt->second.get();
	}

	uint32 RenderProgramInstance::GetBindlessIndex(const RuntimeResource& resource) const
	{
		const SamplerHandle sampler = resource.SamplerHnd.IsValid() ? resource.SamplerHnd : ResourceManager::GetDefaultLinearSampler();
		return resource.TexHandle.GetIndex() | (sampler.GetIndex() << ResourceManager::SAMPLER_INDEX_SHIFT);
	}

	void RenderProgramInstance::BuildPushConstants(size_t passIndex, std::vector<byte>& outData) const
	{
		const ResolvedPass& pass = m_pRenderProgram->GetPasses()[passIndex];
		outData.assign(pass.PushConstantSize, byte{0});

		for (const ResolvedBinding& binding : pass.BufferBindings)
		{
			const std::string&     resolvedName = pass.Ports[binding.PortIndex].ResolvedName;
			const RuntimeResource* pRes         = FindResource(resolvedName);
			if (!pRes || !pRes->IsBuffer())
			{
				POLY_CORE_WARN("Pass '{}': buffer '{}' has no resource bound yet, leaving its push constant pointer as null",
				               pass.Name, resolvedName);
				continue;
			}

			const uint64 address = ResourceManager::Resolve(pRes->BufHandle)->GetDeviceAddress();
			if (binding.Offset + sizeof(uint64) <= outData.size())
				std::memcpy(outData.data() + binding.Offset, &address, sizeof(uint64));
		}

		for (const ResolvedBinding& binding : pass.TextureBindings)
		{
			const std::string&     resolvedName = pass.Ports[binding.PortIndex].ResolvedName;
			const RuntimeResource* pRes         = FindResource(resolvedName);
			if (!pRes || !pRes->TexHandle.IsValid())
			{
				POLY_CORE_WARN("Pass '{}': texture '{}' has no resource bound yet, leaving its push constant handle as 0",
				               pass.Name, resolvedName);
				continue;
			}

			const uint32 heapIndex = GetBindlessIndex(*pRes);
			if (binding.Offset + sizeof(uint32) <= outData.size())
				std::memcpy(outData.data() + binding.Offset, &heapIndex, sizeof(uint32));
		}
	}

	void RenderProgramInstance::ApplyBarrierGroup(CommandBuffer* pCmd, const BarrierGroup& group) const
	{
		if (group.Textures.empty() && group.Buffers.empty())
			return;

		FPipelineStage srcStage = FPipelineStage::NONE;
		FPipelineStage dstStage = FPipelineStage::NONE;

		std::vector<TextureBarrier> textureBarriers;
		textureBarriers.reserve(group.Textures.size());
		for (const auto& t : group.Textures)
		{
			const RuntimeResource* pRes = FindResource(t.ResolvedName);
			if (!pRes || !pRes->IsTexture())
				continue;

			TextureBarrier barrier = {};
			barrier.SrcAccessFlag  = t.SrcAccess;
			barrier.DstAccessFlag  = t.DstAccess;
			barrier.OldLayout      = t.OldLayout;
			barrier.NewLayout      = t.NewLayout;
			barrier.SrcQueueIndex  = 0; // same-queue: equal src/dst collapses to VK_QUEUE_FAMILY_IGNORED
			barrier.DstQueueIndex  = 0;
			barrier.pTexture       = pRes->pTexture;
			barrier.AspectMask     = t.AspectMask;
			textureBarriers.push_back(barrier);

			srcStage |= t.SrcStage;
			dstStage |= t.DstStage;
		}

		std::vector<BufferBarrier> bufferBarriers;
		bufferBarriers.reserve(group.Buffers.size());
		for (const auto& b : group.Buffers)
		{
			const RuntimeResource* pRes = FindResource(b.ResolvedName);
			if (!pRes || !pRes->IsBuffer())
				continue;

			BufferBarrier barrier = {};
			barrier.SrcAccessFlag = b.SrcAccess;
			barrier.DstAccessFlag = b.DstAccess;
			barrier.SrcQueueIndex = 0;
			barrier.DstQueueIndex = 0;
			barrier.pBuffer       = ResourceManager::Resolve(pRes->BufHandle);
			barrier.Offset        = 0;
			bufferBarriers.push_back(barrier);

			srcStage |= b.SrcStage;
			dstStage |= b.DstStage;
		}

		if (textureBarriers.empty() && bufferBarriers.empty())
			return;

		if (srcStage == FPipelineStage::NONE)
			srcStage = FPipelineStage::ALL_COMMANDS;
		if (dstStage == FPipelineStage::NONE)
			dstStage = FPipelineStage::ALL_COMMANDS;

		pCmd->PipelineBarrier(srcStage, dstStage, {}, bufferBarriers, textureBarriers);
	}

	void RenderProgramInstance::ApplyAcquire(CommandBuffer* pCmd, const QueueAcquirePlan& acquire, FQueueType currentQueue) const
	{
		const RuntimeResource* pRes = FindResource(acquire.ResolvedName);
		if (!pRes)
			return;

		const uint32 srcQueueFamily = RenderAPI::GetCommandQueue(acquire.SrcQueue)->GetQueueFamilyIndex();
		const uint32 dstQueueFamily = RenderAPI::GetCommandQueue(currentQueue)->GetQueueFamilyIndex();

		// NOTE: QueueAcquirePlan only carries the consumer's (dst) side - the producer's srcStage isn't
		// available here, so it's approximated as ALL_COMMANDS. Correctness is still guaranteed by the
		// SyncPoint wait issued before this command buffer is submitted; this only affects how tightly
		// scoped the barrier itself is.
		if (acquire.IsTexture)
		{
			if (!pRes->IsTexture())
				return;
			pCmd->AcquireTexture(pRes->pTexture, FPipelineStage::ALL_COMMANDS, acquire.DstStage, acquire.DstAccess,
			                     acquire.OldLayout, acquire.NewLayout, srcQueueFamily, dstQueueFamily);
		}
		else
		{
			if (!pRes->IsBuffer())
				return;
			pCmd->AcquireBuffer(ResourceManager::Resolve(pRes->BufHandle), FPipelineStage::ALL_COMMANDS, acquire.DstStage, acquire.DstAccess, srcQueueFamily, dstQueueFamily);
		}
	}

	void RenderProgramInstance::ApplyRelease(CommandBuffer* pCmd, const QueueReleasePlan& release, FQueueType currentQueue) const
	{
		const RuntimeResource* pRes = FindResource(release.ResolvedName);
		if (!pRes)
			return;

		const uint32 srcQueueFamily = RenderAPI::GetCommandQueue(currentQueue)->GetQueueFamilyIndex();
		const uint32 dstQueueFamily = RenderAPI::GetCommandQueue(release.DstQueue)->GetQueueFamilyIndex();

		if (release.IsTexture)
		{
			if (!pRes->IsTexture())
				return;
			pCmd->ReleaseTexture(pRes->pTexture, release.SrcStage, FPipelineStage::ALL_COMMANDS, release.SrcAccess,
			                     release.OldLayout, release.NewLayout, srcQueueFamily, dstQueueFamily);
		}
		else
		{
			if (!pRes->IsBuffer())
				return;
			pCmd->ReleaseBuffer(ResourceManager::Resolve(pRes->BufHandle), release.SrcStage, FPipelineStage::ALL_COMMANDS, release.SrcAccess, srcQueueFamily, dstQueueFamily);
		}
	}

	// Attachment load op computed at compile time by PlanSynchronization (see RenderProgramBuilder.cpp) -
	// CLEAR on a resource's first write in the program, LOAD on subsequent writes, unless a pass
	// declaration overrode it. Falls back to CLEAR if absent, which shouldn't happen since every
	// $Color/$Depth/$Stencil write port populates this map.
	static ELoadOp GetAttachmentLoadOp(const PassSyncPlan& plan, const std::string& resolvedName)
	{
		auto it = plan.AttachmentLoadOps.find(resolvedName);
		return it != plan.AttachmentLoadOps.end() ? it->second : ELoadOp::CLEAR;
	}

	void RenderProgramInstance::RecordPass(size_t passIndex, const RenderViewContext& view)
	{
		const ResolvedPass&         pass     = m_pRenderProgram->GetPasses()[passIndex];
		const PassSyncPlan&         plan     = m_pRenderProgram->GetSyncPlan().GetPassPlans()[passIndex];
		const PipelineCache::Entry& pipeline = m_PassResources[passIndex].Pipeline;

		CommandBuffer* pCmd = GetCommandBuffer(passIndex);
		pCmd->Begin(FCommandBufferFlag::NONE);

		for (const QueueAcquirePlan& acquire : plan.Acquires)
			ApplyAcquire(pCmd, acquire, pass.Queue);

		ApplyBarrierGroup(pCmd, plan.PreBarriers);

		// Without a pipeline one of the attachments of the pass is missing this frame, only the synchronization
		// the rest of the program relies on is recorded
		if (pipeline.pPipeline)
		{
			// Dynamic rendering: gather this pass's write ports into color/depth/stencil attachments.
			// Only $Color/$Depth/$Stencil are supported as attachments today - PassDeclaration's
			// MapResource() only exposes those three feature ports, so there's no MRT case to handle yet.
			std::vector<RenderingAttachmentInfo> colorAttachments;
			RenderingAttachmentInfo              depthAttachmentInfo   = {};
			RenderingAttachmentInfo              stencilAttachmentInfo = {};
			bool                                 hasDepth              = false;
			bool                                 hasStencil            = false;
			uint32                               width                 = 0;
			uint32                               height                = 0;

			for (const ResolvedPort& port : pass.Ports)
			{
				if (!port.IsWrite || !IsAttachmentSemanticPort(port.Semantic))
					continue;

				const RuntimeResource* pRes = FindResource(port.ResolvedName);

				RenderingAttachmentInfo info = {};
				info.pTextureView            = pRes->pTextureView;
				info.LoadOp                  = GetAttachmentLoadOp(plan, port.ResolvedName);
				info.StoreOp                 = EStoreOp::STORE;

				if (port.Semantic == EFeaturePort::Color)
				{
					info.TextureLayout               = ETextureLayout::COLOR_ATTACHMENT_OPTIMAL;
					info.ClearValue.Color.Float32[3] = 1.0f;
					colorAttachments.push_back(info);

					width  = pRes->pTexture->GetWidth();
					height = pRes->pTexture->GetHeight();
					continue;
				}

				info.TextureLayout                   = ETextureLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				info.ClearValue.DepthStencil.Depth   = 1.0f;
				info.ClearValue.DepthStencil.Stencil = 0;

				if (port.Semantic == EFeaturePort::Depth)
				{
					depthAttachmentInfo = info;
					hasDepth            = true;
				}
				else
				{
					stencilAttachmentInfo = info;
					hasStencil            = true;
				}

				if (width == 0)
				{
					width  = pRes->pTexture->GetWidth();
					height = pRes->pTexture->GetHeight();
				}
			}

			RenderingDesc renderingDesc      = {};
			renderingDesc.RenderWidth        = width;
			renderingDesc.RenderHeight       = height;
			renderingDesc.LayerCount         = 1;
			renderingDesc.ColorAttachments   = colorAttachments;
			renderingDesc.pDepthAttachment   = hasDepth ? &depthAttachmentInfo : nullptr;
			renderingDesc.pStencilAttachment = hasStencil ? &stencilAttachmentInfo : nullptr;

			pCmd->BeginRendering(&renderingDesc);

			pCmd->BindPipeline(pipeline.pPipeline);

			// The view renders to its rect of the target, the whole target if it has none
			const bool hasRect = view.Rect.Width != 0 && view.Rect.Height != 0;

			ViewportDesc viewport = {};
			viewport.PosX         = hasRect ? static_cast<float>(view.Rect.X) : 0.0f;
			viewport.PosY         = hasRect ? static_cast<float>(view.Rect.Y) : 0.0f;
			viewport.Width        = static_cast<float>(hasRect ? view.Rect.Width : width);
			viewport.Height       = static_cast<float>(hasRect ? view.Rect.Height : height);
			pCmd->SetViewport(&viewport);

			ScissorDesc scissor = {};
			scissor.OffsetX     = hasRect ? view.Rect.X : 0;
			scissor.OffsetY     = hasRect ? view.Rect.Y : 0;
			scissor.Width       = hasRect ? view.Rect.Width : width;
			scissor.Height      = hasRect ? view.Rect.Height : height;
			pCmd->SetScissor(&scissor);

			pCmd->BindDescriptor(pipeline.pPipeline, ResourceManager::GetDescriptorSet());

			if (pass.PushConstantSize > 0)
			{
				std::vector<byte> pushData;
				BuildPushConstants(passIndex, pushData);
				pCmd->UpdatePushConstants(pipeline.pLayout, FShaderStage::VERTEX | FShaderStage::FRAGMENT, 0,
				                          static_cast<uint32>(pushData.size()), pushData.data());
			}

			std::vector<DeclaredBuffer> declaredBuffers;
			for (const ResolvedPort& port : pass.Ports)
			{
				if (!port.ShaderName.empty() || port.UsageState == FResourceState::Unknown)
					continue;

				const RuntimeResource* pRes = FindResource(port.ResolvedName);
				if (pRes && pRes->IsBuffer())
					declaredBuffers.push_back({port.Name, ResourceManager::Resolve(pRes->BufHandle)});
			}

			ExecuteContext ctx(pCmd, view, pipeline.pLayout, pass.TextureFields, std::move(declaredBuffers));
			if (pass.ExecuteFn)
				pass.ExecuteFn(ctx);

			pCmd->EndRendering();
		}

		for (const QueueReleasePlan& release : plan.PostReleases)
			ApplyRelease(pCmd, release, pass.Queue);

		ApplyBarrierGroup(pCmd, plan.PostBarriers);

		pCmd->End();
	}
} // namespace Poly
