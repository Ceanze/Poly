#include "RenderProgramInstance.h"

#include "ExecuteContext.h"
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
#include "Poly/Resources/Shader/ShaderManager.h"
#include "RenderView.h"
#include "Resource/ResourceUsage.h"

#include <cstring>

namespace Poly
{
	RenderProgramInstance::RenderProgramInstance(Ref<RenderProgram> pRenderProgram)
	    : m_pRenderProgram(std::move(pRenderProgram))
	{}

	void RenderProgramInstance::Execute(const RenderView& view)
	{
		if (!m_Initialized)
		{
			EnsurePerPassResources();
			m_Initialized = true;
		}

		WaitForFrameSlotReuse(m_FrameIndex);
		ResizeSizedToTargetResources(view);

		const auto& passes    = m_pRenderProgram->GetPasses();
		const auto& passPlans = m_pRenderProgram->GetSyncPlan().GetPassPlans();

		// Phase A: parallel recording
		std::vector<std::function<void()>> tasks;
		tasks.reserve(passes.size());
		for (size_t i = 0; i < passes.size(); i++)
			tasks.push_back([this, i, &view]() { RecordPass(i, view); });

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
			for (const auto& [srcQueue, waitValue] : plan.RequiredWaits)
			{
				SyncPoint* pSrcSyncPoint = GetOrCreateQueueSyncPoint(srcQueue);
				submitDesc.WaitSyncPoints.push_back({pSrcSyncPoint, m_QueueTimelineBase[srcQueue] + waitValue});
			}

			// Acquire any pending uploads for each port resource, handles upload sync and queue acqusition.
			for (const ResolvedPort& port : pass.Ports)
			{
				RuntimeResource* pRes = ResolvePort(port, view);
				if (!pRes)
					continue;

				SyncPoint* pUploadSyncPoint = nullptr;
				uint64     uploadValue      = 0;
				const bool hasPendingUpload = pRes->IsTexture() ? ResourceManager::ConsumePendingUploadSync(pRes->TexHandle, &pUploadSyncPoint, &uploadValue)
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

	void RenderProgramInstance::UpdateResource(std::string_view resolvedName, BufferHandle handle)
	{
		std::lock_guard<std::recursive_mutex> lock(m_ResourcesMutex);
		RuntimeResource&                      res = m_Resources[std::string(resolvedName)];
		res.BufHandle                             = handle;
		res.TexHandle                             = TextureHandle();
		res.SamplerHnd                            = SamplerHandle();
	}

	void RenderProgramInstance::UpdateResource(std::string_view resolvedName, TextureHandle handle, SamplerHandle sampler)
	{
		std::lock_guard<std::recursive_mutex> lock(m_ResourcesMutex);
		RuntimeResource&                      res = m_Resources[std::string(resolvedName)];
		res.BufHandle                             = BufferHandle();
		res.TexHandle                             = handle;
		res.SamplerHnd                            = sampler.IsValid() ? sampler : ResourceManager::GetDefaultLinearSampler();
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

	void RenderProgramInstance::ResizeSizedToTargetResources(const RenderView& view)
	{
		for (auto& [name, res] : m_Resources)
		{
			if (!res.IsSizedToTarget || res.IsBuffer())
				continue;

			TextureDesc desc = ResourceManager::Resolve(res.TexHandle)->GetDesc();
			if (desc.Width != view.pTarget->GetTexture()->GetDesc().Width || desc.Height != view.pTarget->GetTexture()->GetDesc().Height)
			{
				TextureHandle oldHandle = res.TexHandle;
				desc.Width              = view.pTarget->GetTexture()->GetDesc().Width;
				desc.Height             = view.pTarget->GetTexture()->GetDesc().Height;
				res.TexHandle           = ResourceManager::CreateTexture2D(desc.Width, desc.Height, desc.Format, desc.TextureUsage, desc.DebugName);

				ResourceManager::Destroy(oldHandle);
			}
		}
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

	PipelineLayout* RenderProgramInstance::GetOrCreatePipelineLayout(size_t passIndex)
	{
		PerPassResources& res = m_PassResources[passIndex];
		if (res.Layout)
			return res.Layout.get();

		const ResolvedPass& pass = m_pRenderProgram->GetPasses()[passIndex];

		PipelineLayoutDesc desc   = {};
		desc.DescriptorSetLayouts = {ResourceManager::GetSetLayoutDesc()}; // set 0 - shared bindless heap

		if (pass.PushConstantSize > 0)
		{
			PushConstantRange range = {};
			range.ShaderStage       = FShaderStage::VERTEX | FShaderStage::FRAGMENT;
			range.Offset            = 0;
			range.Size              = pass.PushConstantSize;
			desc.PushConstantRanges.push_back(range);
		}

		res.Layout = RenderAPI::CreatePipelineLayout(&desc);
		return res.Layout.get();
	}

	EFormat RenderProgramInstance::GetPortFormat(const ResolvedPort& port, const RenderView& view)
	{
		if (port.ResolvedName == "$Color")
			return view.pTarget ? view.pTarget->GetTexture()->GetDesc().Format : EFormat::R8G8B8A8_UNORM;

		RuntimeResource* pRes = ResolvePort(port, view);
		return (pRes && pRes->IsTexture()) ? ResourceManager::Resolve(pRes->TexHandle)->GetDesc().Format : EFormat::UNDEFINED;
	}

	GraphicsPipeline* RenderProgramInstance::GetOrCreatePipeline(size_t passIndex, const RenderView& view)
	{
		PerPassResources& res = m_PassResources[passIndex];
		if (res.Pipeline)
			return res.Pipeline.get();

		const ResolvedPass& pass = m_pRenderProgram->GetPasses()[passIndex];

		GraphicsPipelineDesc desc = pass.PipelineDesc;
		desc.pPipelineLayout      = GetOrCreatePipelineLayout(passIndex);
		desc.pRenderPass          = nullptr; // dynamic rendering - no VkRenderPass/Framebuffer

		for (const ResolvedPort& port : pass.Ports)
		{
			if (!port.IsWrite)
				continue;

			if (port.ResolvedName == "$Color")
				desc.ColorAttachmentFormats.push_back(GetPortFormat(port, view));
			else if (port.ResolvedName == "$Depth")
				desc.DepthAttachmentFormat = GetPortFormat(port, view);
			else if (port.ResolvedName == "$Stencil")
				desc.StencilAttachmentFormat = GetPortFormat(port, view);
		}

		for (const auto& [shaderPath, shaderStage] : pass.Shaders)
		{
			const PolyID      shaderID   = ShaderManager::CreateShader(shaderPath, shaderStage);
			const ShaderData& shaderData = ShaderManager::GetShader(shaderID);

			if (shaderStage == FShaderStage::VERTEX)
				desc.pVertexShader = shaderData.pShader.get();
			else if (shaderStage == FShaderStage::FRAGMENT)
				desc.pFragmentShader = shaderData.pShader.get();
		}

		res.Pipeline = RenderAPI::CreateGraphicsPipeline(&desc);
		return res.Pipeline.get();
	}

	// Resolves a port's backing resource - never called for "$Color" (always the current frame's
	// view.pTarget, resolved fresh every call, see GetTextureForBarrier/RecordPass - caching it would
	// go stale the moment a different swapchain image comes around). "$Depth"/"$Stencil" and ordinary
	// import/export/global ports ARE cached in m_Resources once allocated.
	RenderProgramInstance::RuntimeResource* RenderProgramInstance::ResolvePort(const ResolvedPort& port, const RenderView& view)
	{
		if (port.ResolvedName == "$Color")
			return nullptr;

		std::lock_guard<std::recursive_mutex> lock(m_ResourcesMutex);

		auto it = m_Resources.find(port.ResolvedName);
		if (it != m_Resources.end())
			return &it->second;

		if (port.IsExternal)
		{
			POLY_CORE_WARN("Resource '{}' has not been supplied via UpdateResource() yet", port.ResolvedName);
			return nullptr;
		}

		const bool isDepthSemantic = port.ResolvedName == "$Depth" || port.ResolvedName == "$Stencil";
		if (!isDepthSemantic && !IsTextureResourceType(port.ResourceType))
		{
			POLY_CORE_ERROR("Resource '{}' is buffer-shaped but not external; graph-owned buffers aren't "
			                "supported yet - supply it via UpdateResource() instead.",
			                port.ResolvedName);
			return nullptr;
		}

		// Graph-owned texture: allocate now, sized either explicitly (WithSize()) or to the render target.
		const bool   isSizedToTarget = (port.Width == 0 || port.Height == 0);
		const uint32 width           = port.Width != 0 ? port.Width : (view.pTarget ? view.pTarget->GetTexture()->GetWidth() : 0);
		const uint32 height          = port.Height != 0 ? port.Height : (view.pTarget ? view.pTarget->GetTexture()->GetHeight() : 0);

		// TODO: IResourceDeclaration has no format setter yet (only WithSize/WithType/WithInitialState) -
		// default until it does; only affects graph-owned internal resources, not externally-supplied ones.
		const EFormat       format = isDepthSemantic ? EFormat::D24_UNORM_S8_UINT : EFormat::R8G8B8A8_UNORM;
		const FTextureUsage usage  = isDepthSemantic ? FTextureUsage::DEPTH_STENCIL_ATTACHMENT | FTextureUsage::SAMPLED
		                                             : FTextureUsage::SAMPLED | (port.ResourceType == EResourceType::StorageImage
		                                                                             ? FTextureUsage::STORAGE
		                                                                             : FTextureUsage::COLOR_ATTACHMENT);

		RuntimeResource res;
		res.TexHandle       = ResourceManager::CreateTexture2D(width, height, format, usage, port.ResolvedName);
		res.SamplerHnd      = ResourceManager::GetDefaultLinearSampler();
		res.IsSizedToTarget = isSizedToTarget;

		auto [insertedIt, inserted] = m_Resources.emplace(port.ResolvedName, std::move(res));
		return &insertedIt->second;
	}

	Texture* RenderProgramInstance::GetTextureForBarrier(const std::string& resolvedName, const RenderView& view)
	{
		if (resolvedName == "$Color")
			return view.pTarget ? view.pTarget->GetTexture() : nullptr;

		std::lock_guard<std::recursive_mutex> lock(m_ResourcesMutex);
		auto                                  it = m_Resources.find(resolvedName);
		return (it != m_Resources.end() && it->second.IsTexture()) ? ResourceManager::Resolve(it->second.TexHandle) : nullptr;
	}

	Buffer* RenderProgramInstance::GetBufferForBarrier(const std::string& resolvedName)
	{
		std::lock_guard<std::recursive_mutex> lock(m_ResourcesMutex);
		auto                                  it = m_Resources.find(resolvedName);
		return (it != m_Resources.end() && it->second.IsBuffer()) ? ResourceManager::Resolve(it->second.BufHandle) : nullptr;
	}

	uint32 RenderProgramInstance::GetBindlessIndex(const RuntimeResource* pResource)
	{
		const SamplerHandle sampler = pResource->SamplerHnd.IsValid() ? pResource->SamplerHnd : ResourceManager::GetDefaultLinearSampler();
		return pResource->TexHandle.GetIndex() | (sampler.GetIndex() << ResourceManager::SAMPLER_INDEX_SHIFT);
	}

	void RenderProgramInstance::BuildPushConstants(size_t passIndex, std::vector<byte>& outData)
	{
		const ResolvedPass& pass = m_pRenderProgram->GetPasses()[passIndex];
		outData.assign(pass.PushConstantSize, byte{0});

		std::lock_guard<std::recursive_mutex> lock(m_ResourcesMutex);

		for (const ResolvedSlot& slot : pass.BufferSlots)
		{
			auto it = m_Resources.find(slot.ResourceName);
			if (it == m_Resources.end() || !it->second.IsBuffer())
			{
				POLY_CORE_WARN("Pass '{}': buffer slot '{}' has no resource bound yet, leaving its bufferAddresses[] slot as 0",
				               pass.Name, slot.ResourceName);
				continue;
			}

			const uint64 address = ResourceManager::Resolve(it->second.BufHandle)->GetDeviceAddress();
			const uint32 offset  = pass.BufferSlotsOffset + slot.Slot * static_cast<uint32>(sizeof(uint64));
			if (offset + sizeof(uint64) <= outData.size())
				std::memcpy(outData.data() + offset, &address, sizeof(uint64));
		}

		for (const ResolvedSlot& slot : pass.TextureSlots)
		{
			auto it = m_Resources.find(slot.ResourceName);
			if (it == m_Resources.end() || !it->second.IsTexture())
			{
				POLY_CORE_WARN("Pass '{}': texture slot '{}' has no resource bound yet, leaving its textureIndices[] slot as 0",
				               pass.Name, slot.ResourceName);
				continue;
			}

			const uint32 heapIndex = GetBindlessIndex(&it->second);
			const uint32 offset    = pass.TextureSlotsOffset + slot.Slot * static_cast<uint32>(sizeof(uint32));
			if (offset + sizeof(uint32) <= outData.size())
				std::memcpy(outData.data() + offset, &heapIndex, sizeof(uint32));
		}
	}

	void RenderProgramInstance::ApplyBarrierGroup(CommandBuffer* pCmd, const BarrierGroup& group, const RenderView& view)
	{
		if (group.Textures.empty() && group.Buffers.empty())
			return;

		FPipelineStage srcStage = FPipelineStage::NONE;
		FPipelineStage dstStage = FPipelineStage::NONE;

		std::vector<TextureBarrier> textureBarriers;
		textureBarriers.reserve(group.Textures.size());
		for (const auto& t : group.Textures)
		{
			Texture* pTexture = GetTextureForBarrier(t.ResolvedName, view);
			if (!pTexture)
				continue;

			TextureBarrier barrier = {};
			barrier.SrcAccessFlag  = t.SrcAccess;
			barrier.DstAccessFlag  = t.DstAccess;
			barrier.OldLayout      = t.OldLayout;
			barrier.NewLayout      = t.NewLayout;
			barrier.SrcQueueIndex  = 0; // same-queue: equal src/dst collapses to VK_QUEUE_FAMILY_IGNORED
			barrier.DstQueueIndex  = 0;
			barrier.pTexture       = pTexture;
			barrier.AspectMask     = t.AspectMask;
			textureBarriers.push_back(barrier);

			srcStage |= t.SrcStage;
			dstStage |= t.DstStage;
		}

		std::vector<BufferBarrier> bufferBarriers;
		bufferBarriers.reserve(group.Buffers.size());
		for (const auto& b : group.Buffers)
		{
			Buffer* pBuffer = GetBufferForBarrier(b.ResolvedName);
			if (!pBuffer)
				continue;

			BufferBarrier barrier = {};
			barrier.SrcAccessFlag = b.SrcAccess;
			barrier.DstAccessFlag = b.DstAccess;
			barrier.SrcQueueIndex = 0;
			barrier.DstQueueIndex = 0;
			barrier.pBuffer       = pBuffer;
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

	void RenderProgramInstance::ApplyAcquire(CommandBuffer* pCmd, const QueueAcquirePlan& acquire, FQueueType currentQueue, const RenderView& view)
	{
		const uint32 srcQueueFamily = RenderAPI::GetCommandQueue(acquire.SrcQueue)->GetQueueFamilyIndex();
		const uint32 dstQueueFamily = RenderAPI::GetCommandQueue(currentQueue)->GetQueueFamilyIndex();

		// NOTE: QueueAcquirePlan only carries the consumer's (dst) side - the producer's srcStage isn't
		// available here, so it's approximated as ALL_COMMANDS. Correctness is still guaranteed by the
		// SyncPoint wait issued before this command buffer is submitted; this only affects how tightly
		// scoped the barrier itself is.
		if (acquire.IsTexture)
		{
			Texture* pTexture = GetTextureForBarrier(acquire.ResolvedName, view);
			if (!pTexture)
				return;
			pCmd->AcquireTexture(pTexture, FPipelineStage::ALL_COMMANDS, acquire.DstStage, acquire.DstAccess,
			                     acquire.OldLayout, acquire.NewLayout, srcQueueFamily, dstQueueFamily);
		}
		else
		{
			Buffer* pBuffer = GetBufferForBarrier(acquire.ResolvedName);
			if (!pBuffer)
				return;
			pCmd->AcquireBuffer(pBuffer, FPipelineStage::ALL_COMMANDS, acquire.DstStage, acquire.DstAccess, srcQueueFamily, dstQueueFamily);
		}
	}

	void RenderProgramInstance::ApplyRelease(CommandBuffer* pCmd, const QueueReleasePlan& release, FQueueType currentQueue, const RenderView& view)
	{
		const uint32 srcQueueFamily = RenderAPI::GetCommandQueue(currentQueue)->GetQueueFamilyIndex();
		const uint32 dstQueueFamily = RenderAPI::GetCommandQueue(release.DstQueue)->GetQueueFamilyIndex();

		if (release.IsTexture)
		{
			Texture* pTexture = GetTextureForBarrier(release.ResolvedName, view);
			if (!pTexture)
				return;
			pCmd->ReleaseTexture(pTexture, release.SrcStage, FPipelineStage::ALL_COMMANDS, release.SrcAccess,
			                     release.OldLayout, release.NewLayout, srcQueueFamily, dstQueueFamily);
		}
		else
		{
			Buffer* pBuffer = GetBufferForBarrier(release.ResolvedName);
			if (!pBuffer)
				return;
			pCmd->ReleaseBuffer(pBuffer, release.SrcStage, FPipelineStage::ALL_COMMANDS, release.SrcAccess, srcQueueFamily, dstQueueFamily);
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

	void RenderProgramInstance::RecordPass(size_t passIndex, const RenderView& view)
	{
		const ResolvedPass& pass = m_pRenderProgram->GetPasses()[passIndex];
		const PassSyncPlan& plan = m_pRenderProgram->GetSyncPlan().GetPassPlans()[passIndex];

		// Resolve every port's backing resource up front (lazy internal allocation / external lookup) -
		// pipeline creation and push-constant packing below both need these already resolved.
		for (const ResolvedPort& port : pass.Ports)
			if (port.ResolvedName != "$Color")
				ResolvePort(port, view);

		CommandBuffer* pCmd = GetCommandBuffer(passIndex);
		pCmd->Begin(FCommandBufferFlag::NONE);

		for (const QueueAcquirePlan& acquire : plan.Acquires)
			ApplyAcquire(pCmd, acquire, pass.Queue, view);

		ApplyBarrierGroup(pCmd, plan.PreBarriers, view);

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
			if (!port.IsWrite)
				continue;

			if (port.ResolvedName == "$Color")
			{
				RenderingAttachmentInfo info     = {};
				info.pTextureView                = view.pTarget;
				info.TextureLayout               = ETextureLayout::COLOR_ATTACHMENT_OPTIMAL;
				info.LoadOp                      = GetAttachmentLoadOp(plan, port.ResolvedName);
				info.StoreOp                     = EStoreOp::STORE;
				info.ClearValue.Color.Float32[3] = 1.0f;
				colorAttachments.push_back(info);

				if (view.pTarget)
				{
					width  = view.pTarget->GetTexture()->GetWidth();
					height = view.pTarget->GetTexture()->GetHeight();
				}
			}
			else if (port.ResolvedName == "$Depth" || port.ResolvedName == "$Stencil")
			{
				RuntimeResource* pRes = ResolvePort(port, view);
				if (!pRes)
					continue;

				RenderingAttachmentInfo info         = {};
				info.pTextureView                    = ResourceManager::ResolveView(pRes->TexHandle);
				info.TextureLayout                   = ETextureLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				info.LoadOp                          = GetAttachmentLoadOp(plan, port.ResolvedName);
				info.StoreOp                         = EStoreOp::STORE;
				info.ClearValue.DepthStencil.Depth   = 1.0f;
				info.ClearValue.DepthStencil.Stencil = 0;

				if (port.ResolvedName == "$Depth")
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
					Texture* pDepthTexture = ResourceManager::Resolve(pRes->TexHandle);
					width                  = pDepthTexture->GetWidth();
					height                 = pDepthTexture->GetHeight();
				}
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

		GraphicsPipeline* pPipeline = GetOrCreatePipeline(passIndex, view);
		pCmd->BindPipeline(pPipeline);

		ViewportDesc viewport = {};
		viewport.Width        = static_cast<float>(width);
		viewport.Height       = static_cast<float>(height);
		pCmd->SetViewport(&viewport);

		ScissorDesc scissor = {};
		scissor.Width       = width;
		scissor.Height      = height;
		pCmd->SetScissor(&scissor);

		pCmd->BindDescriptor(pPipeline, ResourceManager::GetDescriptorSet());

		if (pass.PushConstantSize > 0)
		{
			std::vector<byte> pushData;
			BuildPushConstants(passIndex, pushData);
			pCmd->UpdatePushConstants(GetOrCreatePipelineLayout(passIndex), FShaderStage::VERTEX | FShaderStage::FRAGMENT, 0,
			                          static_cast<uint32>(pushData.size()), pushData.data());
		}

		ExecuteContext ctx(pCmd, view, GetOrCreatePipelineLayout(passIndex), pass.TextureSlotsOffset);
		if (pass.ExecuteFn)
			pass.ExecuteFn(ctx);

		pCmd->EndRendering();

		for (const QueueReleasePlan& release : plan.PostReleases)
			ApplyRelease(pCmd, release, pass.Queue, view);

		ApplyBarrierGroup(pCmd, plan.PostBarriers, view);

		pCmd->End();
	}
} // namespace Poly
