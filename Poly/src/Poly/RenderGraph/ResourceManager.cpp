#include "ResourceManager.h"

#include "Platform/API/CommandBuffer.h"
#include "Platform/API/CommandPool.h"
#include "Platform/API/CommandQueue.h"
#include "Platform/API/DescriptorSet.h"
#include "Platform/API/SyncPoint.h"
#include "Poly/Core/RenderAPI.h"

#include <algorithm>
#include <cstring>
#include <tuple>
#include <unordered_set>

namespace Poly
{
	void ResourceManager::Init()
	{
		// Note: Vulkan only allows VARIABLE_DESCRIPTOR_COUNT on the single highest-numbered binding in
		// a set, and this set has two array bindings - so both are declared with a fixed
		// MAX_TEXTURES/MAX_SAMPLERS count at layout-creation time instead (PARTIALLY_BOUND still lets
		// unused slots stay unwritten, UPDATE_AFTER_BIND still allows registering more while in flight).
		DescriptorSetBinding texturesBinding = {};
		texturesBinding.Binding              = 0;
		texturesBinding.DescriptorType       = EDescriptorType::SAMPLED_IMAGE;
		texturesBinding.DescriptorCount      = MAX_TEXTURES;
		texturesBinding.ShaderStage          = FShaderStage::VERTEX | FShaderStage::FRAGMENT;
		texturesBinding.BindingFlags         = FDescriptorIndexingBindingFlag::PARTIALLY_BOUND | FDescriptorIndexingBindingFlag::UPDATE_AFTER_BIND;

		DescriptorSetBinding samplersBinding = {};
		samplersBinding.Binding              = 1;
		samplersBinding.DescriptorType       = EDescriptorType::SAMPLER;
		samplersBinding.DescriptorCount      = MAX_SAMPLERS;
		samplersBinding.ShaderStage          = FShaderStage::VERTEX | FShaderStage::FRAGMENT;
		samplersBinding.BindingFlags         = FDescriptorIndexingBindingFlag::PARTIALLY_BOUND | FDescriptorIndexingBindingFlag::UPDATE_AFTER_BIND;

		s_SetLayoutDesc.DescriptorSetBindings = {texturesBinding, samplersBinding};

		PipelineLayoutDesc layoutDesc   = {};
		layoutDesc.DescriptorSetLayouts = {s_SetLayoutDesc};
		s_pHeapPipelineLayout           = RenderAPI::CreatePipelineLayout(&layoutDesc);

		s_pHeapSet = RenderAPI::CreateDescriptorSet(s_pHeapPipelineLayout.get(), 0);

		for (uint32 i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			s_TransferCommandPools[i]   = RenderAPI::CreateCommandPool(FQueueType::TRANSFER, FCommandPoolFlags::NONE);
			s_TransferCommandBuffers[i] = s_TransferCommandPools[i]->AllocateCommandBuffer(ECommandBufferLevel::PRIMARY);
		}
		s_pUploadSyncPoint = RenderAPI::CreateSyncPoint();

		s_DefaultLinearSampler  = GetOrCreateSampler(Sampler::GetDefaultLinearSampler()->GetDesc());
		s_DefaultNearestSampler = GetOrCreateSampler(Sampler::GetDefaultNearestSampler()->GetDesc());
	}

	void ResourceManager::Release()
	{
		RenderAPI::GetCommandQueue(FQueueType::TRANSFER)->Wait();
		RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Wait();
		for (const auto& [queue, ring] : s_AcquireRings)
			RenderAPI::GetCommandQueue(queue)->Wait();

		for (uint32 i = 0; i < FRAMES_IN_FLIGHT; i++)
			s_TransferCommandPools[i].reset();
		s_AcquireRings.clear();
		s_pUploadSyncPoint.reset();
		s_UploadTimelineValue = 0;
		s_SlotSignalValue     = {};

		for (uint32 i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			if (s_StagingBuffers[i])
				s_StagingBuffers[i]->Unmap();
			s_StagingBuffers[i].reset();
		}
		s_StagingBufferCapacity = {};
		s_StagingBufferMapped   = {};

		s_Textures.clear();
		s_FreeTextureIndices.clear();
		s_Buffers.clear();
		s_FreeBufferIndices.clear();
		s_Samplers.clear();
		s_SamplerCache.clear();
		s_PendingTextureUploads.clear();
		s_PendingBufferUploads.clear();
		s_PendingBufferCopies.clear();
		s_PendingTextureDestroys.clear();
		s_PendingBufferDestroys.clear();

		s_pHeapSet.reset();
		s_pHeapPipelineLayout.reset();
	}

	bool ResourceManager::SamplerDescLess::operator()(const SamplerDesc& a, const SamplerDesc& b) const
	{
		const auto AsTuple = [](const SamplerDesc& d) {
			return std::tie(d.MinFilter, d.MagFilter, d.AddressModeU, d.AddressModeV, d.AddressModeW, d.MipMapMode,
			                d.BorderColor, d.MipLodBias, d.MinLod, d.MaxLod, d.AnistropyEnable, d.MaxAnisotropy);
		};
		return AsTuple(a) < AsTuple(b);
	}

	uint32 ResourceManager::AllocTextureSlot()
	{
		if (!s_FreeTextureIndices.empty())
		{
			uint32 index = s_FreeTextureIndices.back();
			s_FreeTextureIndices.pop_back();
			return index;
		}

		POLY_VALIDATE(s_Textures.size() < MAX_TEXTURES, "Bindless texture heap is full ({} textures)", MAX_TEXTURES);
		s_Textures.emplace_back();
		return static_cast<uint32>(s_Textures.size() - 1);
	}

	uint32 ResourceManager::AllocBufferSlot()
	{
		if (!s_FreeBufferIndices.empty())
		{
			uint32 index = s_FreeBufferIndices.back();
			s_FreeBufferIndices.pop_back();
			return index;
		}

		s_Buffers.emplace_back();
		return static_cast<uint32>(s_Buffers.size() - 1);
	}

	TextureHandle ResourceManager::CreateTexture2D(uint32 width, uint32 height, EFormat format, FTextureUsage usage, std::string debugName)
	{
		const bool isDepth = BitsSet(FTextureUsage::DEPTH_STENCIL_ATTACHMENT, usage);

		TextureDesc texDesc  = {};
		texDesc.Width        = width;
		texDesc.Height       = height;
		texDesc.Depth        = 1;
		texDesc.ArrayLayers  = 1;
		texDesc.MipLevels    = 1;
		texDesc.SampleCount  = 1;
		texDesc.MemoryUsage  = EMemoryUsage::GPU_ONLY;
		texDesc.Format       = format;
		texDesc.TextureDim   = ETextureDim::DIM_2D;
		texDesc.TextureUsage = usage | FTextureUsage::TRANSFER_DST;
		texDesc.DebugName    = debugName;

		Ref<Texture> pTexture = RenderAPI::CreateTexture(&texDesc);

		TextureViewDesc viewDesc = {};
		viewDesc.pTexture        = pTexture.get();
		viewDesc.ImageViewType   = EImageViewType::TYPE_2D;
		viewDesc.Format          = format;
		viewDesc.ImageViewFlag   = isDepth ? FImageViewFlag::DEPTH_STENCIL : FImageViewFlag::COLOR;
		viewDesc.MipLevelCount   = 1;
		viewDesc.ArrayLayerCount = 1;
		viewDesc.DebugName       = debugName;

		Ref<TextureView> pView = RenderAPI::CreateTextureView(&viewDesc);

		std::lock_guard<std::recursive_mutex> lock(s_Mutex);
		uint32                                index = AllocTextureSlot();
		TextureSlot&                          slot  = s_Textures[index];
		slot.pTexture                               = pTexture;
		slot.pDefaultView                           = pView;
		slot.Width                                  = width;
		slot.Height                                 = height;
		slot.Format                                 = format;
		slot.DebugName                              = std::move(debugName);
		slot.Alive                                  = true;

		// TODO: Allow depth stencil support
		// Depth/stencil textures are skipped: their view has both aspects set (valid for use as an
		// attachment, but Vulkan forbids a combined depth+stencil view in a SAMPLED_IMAGE descriptor -
		// VUID-VkDescriptorImageInfo-imageView-01976). Nothing samples depth/stencil resources through
		// the bindless heap today (RenderProgramInstance never calls GetBindlessIndex() for $Depth/
		// $Stencil ports), so leaving their slot unwritten is safe - PARTIALLY_BOUND allows that.
		if (!isDepth)
			s_pHeapSet->UpdateTextureBinding(0, ETextureLayout::SHADER_READ_ONLY_OPTIMAL, pView.get(), nullptr, index);

		return TextureHandle(index, slot.Generation);
	}

	BufferHandle ResourceManager::CreateBuffer(uint64 size, FBufferUsage usage, EMemoryUsage memUsage, std::string debugName)
	{
		BufferDesc desc  = {};
		desc.Size        = size;
		desc.MemUsage    = memUsage;
		desc.BufferUsage = usage | (memUsage == EMemoryUsage::GPU_ONLY ? FBufferUsage::TRANSFER_DST : FBufferUsage::NONE);

		Ref<Buffer> pBuffer = RenderAPI::CreateBuffer(&desc);

		std::lock_guard<std::recursive_mutex> lock(s_Mutex);
		uint32                                index = AllocBufferSlot();
		BufferSlot&                           slot  = s_Buffers[index];
		slot.pBuffer                                = pBuffer;
		slot.DebugName                              = std::move(debugName);
		slot.Alive                                  = true;

		return BufferHandle(index, slot.Generation);
	}

	BufferHandle ResourceManager::CreateUniformBuffer(uint64 size, std::string debugName)
	{
		return CreateBuffer(size, FBufferUsage::UNIFORM_BUFFER | FBufferUsage::SHADER_DEVICE_ADDRESS, EMemoryUsage::CPU_VISIBLE, std::move(debugName));
	}

	BufferHandle ResourceManager::CreateVertexBuffer(uint64 size, EMemoryUsage memUsage, std::string debugName)
	{
		return CreateBuffer(size, FBufferUsage::VERTEX_BUFFER, memUsage, std::move(debugName));
	}

	BufferHandle ResourceManager::CreateIndexBuffer(uint64 size, EMemoryUsage memUsage, std::string debugName)
	{
		return CreateBuffer(size, FBufferUsage::INDEX_BUFFER, memUsage, std::move(debugName));
	}

	BufferHandle ResourceManager::CreateStorageBuffer(uint64 size, EMemoryUsage memUsage, std::string debugName)
	{
		return CreateBuffer(size, FBufferUsage::STORAGE_BUFFER | FBufferUsage::SHADER_DEVICE_ADDRESS, memUsage, std::move(debugName));
	}

	BufferHandle ResourceManager::ResizeBuffer(BufferHandle handle, uint64 newSize, FQueueType targetQueue, uint64 preserveBytes)
	{
		std::lock_guard<std::recursive_mutex> lock(s_Mutex);

		if (!handle.IsValid() || handle.GetIndex() >= s_Buffers.size())
		{
			POLY_CORE_WARN("ResizeBuffer: invalid BufferHandle");
			return {};
		}

		BufferSlot& oldSlot = s_Buffers[handle.GetIndex()];
		if (!oldSlot.Alive || oldSlot.Generation != handle.GetGeneration())
		{
			POLY_CORE_WARN("ResizeBuffer: stale BufferHandle");
			return {};
		}

		// Resolve any uploads still queued against the old buffer before its contents are copied
		// forward below - otherwise they'd be silently lost once the old slot is torn down.
		FlushUploads();

		const BufferDesc  oldDesc   = oldSlot.pBuffer->GetDesc();
		const std::string debugName = oldSlot.DebugName;
		POLY_VALIDATE(BitsSet(oldDesc.BufferUsage, FBufferUsage::TRANSFER_SRC),
		              "ResizeBuffer: '{}' was not created with TRANSFER_SRC usage - cannot copy its contents forward", debugName);

		BufferDesc newDesc     = oldDesc;
		newDesc.Size           = newSize;
		Ref<Buffer> pNewBuffer = RenderAPI::CreateBuffer(&newDesc);

		const uint32 newIndex = AllocBufferSlot();
		BufferSlot&  newSlot  = s_Buffers[newIndex];
		newSlot.pBuffer       = pNewBuffer;
		newSlot.DebugName     = debugName;
		newSlot.Alive         = true;

		const BufferHandle newHandle(newIndex, newSlot.Generation);

		PendingBufferCopy copy;
		copy.SrcHandle   = handle;
		copy.DstHandle   = newHandle;
		copy.Size        = std::min({oldDesc.Size, newSize, preserveBytes});
		copy.TargetQueue = targetQueue;
		s_PendingBufferCopies.push_back(copy);

		// Mark as dead, it will be queued for destruction once the copy completes
		s_Buffers[handle.GetIndex()].Alive = false;

		return newHandle;
	}

	Ref<TextureView> ResourceManager::CreateCustomView(TextureHandle texture, TextureViewDesc desc)
	{
		Texture* pTexture = Resolve(texture);
		if (!pTexture)
		{
			POLY_CORE_WARN("CreateCustomView: stale or invalid TextureHandle");
			return nullptr;
		}

		desc.pTexture = pTexture;
		return RenderAPI::CreateTextureView(&desc);
	}

	uint32 ResourceManager::RegisterSamplerIndex(Sampler* pSampler, Ref<Sampler> pOwnedRef)
	{
		auto it = s_SamplerCache.find(pSampler->GetDesc());
		if (it != s_SamplerCache.end())
			return it->second.GetIndex();

		POLY_VALIDATE(s_Samplers.size() < MAX_SAMPLERS, "Bindless sampler heap is full ({} samplers)", MAX_SAMPLERS);

		const uint32 index = static_cast<uint32>(s_Samplers.size());
		s_Samplers.push_back(std::move(pOwnedRef)); // null for externally-registered samplers
		s_pHeapSet->UpdateSamplerBinding(1, pSampler, index);
		s_SamplerCache.emplace(pSampler->GetDesc(), SamplerHandle(index, 0));
		return index;
	}

	SamplerHandle ResourceManager::GetOrCreateSampler(const SamplerDesc& desc)
	{
		std::lock_guard<std::recursive_mutex> lock(s_Mutex);

		auto it = s_SamplerCache.find(desc);
		if (it != s_SamplerCache.end())
			return it->second;

		Ref<Sampler> pSampler = RenderAPI::CreateSampler(&desc);
		const uint32 index    = RegisterSamplerIndex(pSampler.get(), pSampler);
		return SamplerHandle(index, 0);
	}

	SamplerHandle ResourceManager::GetDefaultLinearSampler()
	{
		return s_DefaultLinearSampler;
	}

	SamplerHandle ResourceManager::GetDefaultNearestSampler()
	{
		return s_DefaultNearestSampler;
	}

	uint32 ResourceManager::RegisterExternalTextureAndSampler(const TextureView* pTextureView, ETextureLayout layout, Sampler* pSampler)
	{
		std::lock_guard<std::recursive_mutex> lock(s_Mutex);

		const uint32 textureIndex      = AllocTextureSlot();
		s_Textures[textureIndex].Alive = true; // pTexture/pDefaultView stay null - not manager-owned
		s_pHeapSet->UpdateTextureBinding(0, layout, pTextureView, nullptr, textureIndex);

		const uint32 samplerIndex = RegisterSamplerIndex(pSampler, nullptr);

		return textureIndex | (samplerIndex << SAMPLER_INDEX_SHIFT);
	}

	Texture* ResourceManager::Resolve(TextureHandle handle)
	{
		if (!handle.IsValid())
			return nullptr;

		std::lock_guard<std::recursive_mutex> lock(s_Mutex);
		if (handle.GetIndex() >= s_Textures.size())
			return nullptr;

		TextureSlot& slot = s_Textures[handle.GetIndex()];
		if (!slot.Alive || slot.Generation != handle.GetGeneration())
			return nullptr;

		return slot.pTexture.get();
	}

	TextureView* ResourceManager::ResolveView(TextureHandle handle)
	{
		if (!handle.IsValid())
			return nullptr;

		std::lock_guard<std::recursive_mutex> lock(s_Mutex);
		if (handle.GetIndex() >= s_Textures.size())
			return nullptr;

		TextureSlot& slot = s_Textures[handle.GetIndex()];
		if (!slot.Alive || slot.Generation != handle.GetGeneration())
			return nullptr;

		return slot.pDefaultView.get();
	}

	Buffer* ResourceManager::Resolve(BufferHandle handle)
	{
		if (!handle.IsValid())
			return nullptr;

		std::lock_guard<std::recursive_mutex> lock(s_Mutex);
		if (handle.GetIndex() >= s_Buffers.size())
			return nullptr;

		BufferSlot& slot = s_Buffers[handle.GetIndex()];
		if (!slot.Alive || slot.Generation != handle.GetGeneration())
			return nullptr;

		return slot.pBuffer.get();
	}

	Sampler* ResourceManager::Resolve(SamplerHandle handle)
	{
		if (!handle.IsValid())
			return nullptr;

		std::lock_guard<std::recursive_mutex> lock(s_Mutex);
		if (handle.GetIndex() >= s_Samplers.size())
			return nullptr;

		return s_Samplers[handle.GetIndex()].get();
	}

	void ResourceManager::Destroy(TextureHandle handle)
	{
		std::lock_guard<std::recursive_mutex> lock(s_Mutex);
		if (!handle.IsValid() || handle.GetIndex() >= s_Textures.size())
			return;

		TextureSlot& slot = s_Textures[handle.GetIndex()];
		if (!slot.Alive || slot.Generation != handle.GetGeneration())
			return;

		slot.Alive = false;
		s_PendingTextureDestroys.push_back({handle.GetIndex(), s_CurrentFrame});
	}

	void ResourceManager::Destroy(BufferHandle handle)
	{
		std::lock_guard<std::recursive_mutex> lock(s_Mutex);
		if (!handle.IsValid() || handle.GetIndex() >= s_Buffers.size())
			return;

		BufferSlot& slot = s_Buffers[handle.GetIndex()];
		if (!slot.Alive || slot.Generation != handle.GetGeneration())
			return;

		slot.Alive = false;
		EnqueueBufferDestroy(handle.GetIndex());
	}

	void ResourceManager::EnqueueBufferDestroy(uint32 index, uint64 requiredSyncValue)
	{
		s_PendingBufferDestroys.push_back({index, s_CurrentFrame, requiredSyncValue});
	}

	void ResourceManager::UploadTextureData(TextureHandle handle, const void* pData, uint32 width, uint32 height, FQueueType targetQueue)
	{
		std::lock_guard<std::recursive_mutex> lock(s_Mutex);
		if (!handle.IsValid() || handle.GetIndex() >= s_Textures.size() || s_Textures[handle.GetIndex()].Generation != handle.GetGeneration())
		{
			POLY_CORE_WARN("UploadTextureData: stale or invalid TextureHandle");
			return;
		}

		// TODO: assumes 4 bytes/pixel (RGBA8), same simplification AssetLoader::LoadTextureFromMemory
		// makes today - format/channels should determine the real stride instead.
		PendingTextureUpload upload;
		upload.Handle      = handle;
		upload.Width       = width;
		upload.Height      = height;
		upload.TargetQueue = targetQueue;
		upload.Data.assign(static_cast<const byte*>(pData), static_cast<const byte*>(pData) + static_cast<size_t>(width) * height * 4);
		s_PendingTextureUploads.push_back(std::move(upload));
	}

	void ResourceManager::UploadBufferData(BufferHandle handle, const void* pData, uint64 size, uint64 offset, FQueueType targetQueue)
	{
		std::lock_guard<std::recursive_mutex> lock(s_Mutex);
		if (!handle.IsValid() || handle.GetIndex() >= s_Buffers.size())
		{
			POLY_CORE_WARN("UploadBufferData: invalid BufferHandle");
			return;
		}

		BufferSlot& slot = s_Buffers[handle.GetIndex()];
		if (!slot.Alive || slot.Generation != handle.GetGeneration())
		{
			POLY_CORE_WARN("UploadBufferData: stale BufferHandle");
			return;
		}

		if (slot.pBuffer->GetDesc().MemUsage != EMemoryUsage::GPU_ONLY)
		{
			// Host-visible - write directly, no staging/flush needed.
			slot.pBuffer->TransferData(pData, static_cast<size_t>(size), static_cast<size_t>(offset));
			return;
		}

		PendingBufferUpload upload;
		upload.Handle      = handle;
		upload.Offset      = offset;
		upload.TargetQueue = targetQueue;
		upload.Data.assign(static_cast<const byte*>(pData), static_cast<const byte*>(pData) + size);
		s_PendingBufferUploads.push_back(std::move(upload));
	}

	bool ResourceManager::ConsumePendingUploadSync(TextureHandle handle, SyncPoint** ppSyncPoint, uint64* pValue)
	{
		std::lock_guard<std::recursive_mutex> lock(s_Mutex);
		if (!handle.IsValid() || handle.GetIndex() >= s_Textures.size())
			return false;

		TextureSlot& slot = s_Textures[handle.GetIndex()];
		if (!slot.Alive || slot.Generation != handle.GetGeneration() || slot.PendingUploadValue == 0)
			return false;

		*ppSyncPoint            = s_pUploadSyncPoint.get();
		*pValue                 = slot.PendingUploadValue;
		slot.PendingUploadValue = 0;
		return true;
	}

	bool ResourceManager::ConsumePendingUploadSync(BufferHandle handle, SyncPoint** ppSyncPoint, uint64* pValue)
	{
		std::lock_guard<std::recursive_mutex> lock(s_Mutex);
		if (!handle.IsValid() || handle.GetIndex() >= s_Buffers.size())
			return false;

		BufferSlot& slot = s_Buffers[handle.GetIndex()];
		if (!slot.Alive || slot.Generation != handle.GetGeneration() || slot.PendingUploadValue == 0)
			return false;

		*ppSyncPoint            = s_pUploadSyncPoint.get();
		*pValue                 = slot.PendingUploadValue;
		slot.PendingUploadValue = 0;
		return true;
	}

	void ResourceManager::EnsureStagingCapacity(uint32 slot, uint64 requiredSize)
	{
		if (s_StagingBuffers[slot] && s_StagingBufferCapacity[slot] >= requiredSize)
			return;

		if (s_StagingBuffers[slot])
			s_StagingBuffers[slot]->Unmap();

		const uint64 newCapacity = s_StagingBufferCapacity[slot] == 0 ? requiredSize : std::max(requiredSize, s_StagingBufferCapacity[slot] * 2);

		BufferDesc stagingDesc  = {};
		stagingDesc.BufferUsage = FBufferUsage::TRANSFER_SRC;
		stagingDesc.MemUsage    = EMemoryUsage::CPU_VISIBLE;
		stagingDesc.Size        = newCapacity;

		s_StagingBuffers[slot]        = RenderAPI::CreateBuffer(&stagingDesc);
		s_StagingBufferCapacity[slot] = newCapacity;
		s_StagingBufferMapped[slot]   = s_StagingBuffers[slot]->Map();
	}

	ResourceManager::QueueCommandRing& ResourceManager::GetOrCreateAcquireRing(FQueueType queue)
	{
		auto it = s_AcquireRings.find(queue);
		if (it != s_AcquireRings.end())
			return it->second;

		QueueCommandRing ring;
		for (uint32 i = 0; i < FRAMES_IN_FLIGHT; i++)
		{
			ring.Pools[i]   = RenderAPI::CreateCommandPool(queue, FCommandPoolFlags::NONE);
			ring.Buffers[i] = ring.Pools[i]->AllocateCommandBuffer(ECommandBufferLevel::PRIMARY);
		}

		auto [insertedIt, inserted] = s_AcquireRings.emplace(queue, std::move(ring));
		return insertedIt->second;
	}

	void ResourceManager::FlushUploads()
	{
		if (s_PendingTextureUploads.empty() && s_PendingBufferUploads.empty() && s_PendingBufferCopies.empty())
			return;

		const uint32 slot = static_cast<uint32>(s_CurrentFrame % FRAMES_IN_FLIGHT);

		if (s_SlotSignalValue[slot] > 0)
			s_pUploadSyncPoint->Wait(s_SlotSignalValue[slot]);

		uint64 totalStagingSize = 0;
		for (const auto& upload : s_PendingTextureUploads)
			totalStagingSize += upload.Data.size();
		for (const auto& upload : s_PendingBufferUploads)
			totalStagingSize += upload.Data.size();

		EnsureStagingCapacity(slot, totalStagingSize);
		byte*  pMapped = static_cast<byte*>(s_StagingBufferMapped[slot]);
		uint64 offset  = 0;

		const uint32 transferFamily = RenderAPI::GetCommandQueue(FQueueType::TRANSFER)->GetQueueFamilyIndex();

		std::unordered_map<FQueueType, std::vector<TextureHandle>> texturesByTarget;
		std::unordered_map<FQueueType, std::vector<BufferHandle>>  buffersByTarget;

		CommandPool*   pTransferPool = s_TransferCommandPools[slot].get();
		CommandBuffer* pTransferCmd  = s_TransferCommandBuffers[slot];

		pTransferPool->Reset();
		pTransferCmd->Begin(FCommandBufferFlag::ONE_TIME_SUBMIT);

		for (const auto& upload : s_PendingTextureUploads)
		{
			memcpy(pMapped + offset, upload.Data.data(), upload.Data.size());
			Texture* pTexture = s_Textures[upload.Handle.GetIndex()].pTexture.get();

			pTransferCmd->PipelineTextureBarrier(pTexture, FPipelineStage::ALL_COMMANDS, FPipelineStage::TRANSFER, FAccessFlag::NONE,
			                                     FAccessFlag::TRANSFER_WRITE, ETextureLayout::UNDEFINED, ETextureLayout::TRANSFER_DST_OPTIMAL);

			CopyBufferDesc copyDesc = {};
			copyDesc.BufferOffset   = offset;
			copyDesc.Width          = upload.Width;
			copyDesc.Height         = upload.Height;
			copyDesc.Depth          = 1;
			copyDesc.ArrayCount     = 1;
			pTransferCmd->CopyBufferToTexture(s_StagingBuffers[slot].get(), pTexture, ETextureLayout::TRANSFER_DST_OPTIMAL, copyDesc);

			const uint32 targetFamily = RenderAPI::GetCommandQueue(upload.TargetQueue)->GetQueueFamilyIndex();
			if (targetFamily != transferFamily)
			{
				pTransferCmd->ReleaseTexture(pTexture, FPipelineStage::TRANSFER, FPipelineStage::TRANSFER, FAccessFlag::TRANSFER_READ,
				                             ETextureLayout::TRANSFER_DST_OPTIMAL, ETextureLayout::SHADER_READ_ONLY_OPTIMAL, transferFamily,
				                             targetFamily);
				texturesByTarget[upload.TargetQueue].push_back(upload.Handle);
			}

			offset += upload.Data.size();
		}

		for (const auto& upload : s_PendingBufferUploads)
		{
			memcpy(pMapped + offset, upload.Data.data(), upload.Data.size());
			Buffer* pBuffer = s_Buffers[upload.Handle.GetIndex()].pBuffer.get();

			pTransferCmd->CopyBuffer(s_StagingBuffers[slot].get(), pBuffer, upload.Data.size(), offset, upload.Offset);

			const uint32 targetFamily = RenderAPI::GetCommandQueue(upload.TargetQueue)->GetQueueFamilyIndex();
			if (targetFamily != transferFamily)
			{
				pTransferCmd->ReleaseBuffer(pBuffer, FPipelineStage::TRANSFER, FPipelineStage::TRANSFER, FAccessFlag::TRANSFER_READ,
				                            transferFamily, targetFamily);
				buffersByTarget[upload.TargetQueue].push_back(upload.Handle);
			}

			offset += upload.Data.size();
		}

		// Buffer-to-buffer copies
		for (const auto& copy : s_PendingBufferCopies)
		{
			Buffer* pSrcBuffer = s_Buffers[copy.SrcHandle.GetIndex()].pBuffer.get();
			Buffer* pDstBuffer = s_Buffers[copy.DstHandle.GetIndex()].pBuffer.get();

			pTransferCmd->CopyBuffer(pSrcBuffer, pDstBuffer, copy.Size, 0, 0);

			const uint32 targetFamily = RenderAPI::GetCommandQueue(copy.TargetQueue)->GetQueueFamilyIndex();
			if (targetFamily != transferFamily)
			{
				pTransferCmd->ReleaseBuffer(pDstBuffer, FPipelineStage::TRANSFER, FPipelineStage::TRANSFER, FAccessFlag::TRANSFER_READ,
				                            transferFamily, targetFamily);
				buffersByTarget[copy.TargetQueue].push_back(copy.DstHandle);
			}
		}

		pTransferCmd->End();

		const uint64 transferSignalValue = ++s_UploadTimelineValue;
		SubmitDesc   transferSubmit      = {};
		transferSubmit.CommandBuffers    = {pTransferCmd};
		transferSubmit.SignalSyncPoints  = {{s_pUploadSyncPoint.get(), transferSignalValue}};
		RenderAPI::GetCommandQueue(FQueueType::TRANSFER)->Submit(transferSubmit);

		uint64 highestSignalValue = transferSignalValue;

		std::unordered_set<FQueueType> targets;
		for (const auto& [queue, uploads] : texturesByTarget)
			targets.insert(queue);
		for (const auto& [queue, uploads] : buffersByTarget)
			targets.insert(queue);

		for (FQueueType target : targets)
		{
			QueueCommandRing& ring = GetOrCreateAcquireRing(target);
			ring.Pools[slot]->Reset();
			ring.Buffers[slot]->Begin(FCommandBufferFlag::ONE_TIME_SUBMIT);

			const uint32 targetFamily = RenderAPI::GetCommandQueue(target)->GetQueueFamilyIndex();

			for (const auto& handle : texturesByTarget[target])
			{
				Texture* pTexture = s_Textures[handle.GetIndex()].pTexture.get();
				ring.Buffers[slot]->AcquireTexture(pTexture, FPipelineStage::TRANSFER, FPipelineStage::TRANSFER, FAccessFlag::TRANSFER_READ,
				                                   ETextureLayout::TRANSFER_DST_OPTIMAL, ETextureLayout::SHADER_READ_ONLY_OPTIMAL, transferFamily,
				                                   targetFamily);
			}
			for (const auto& bufferHandle : buffersByTarget[target])
			{
				Buffer* pBuffer = s_Buffers[bufferHandle.GetIndex()].pBuffer.get();
				ring.Buffers[slot]->AcquireBuffer(pBuffer, FPipelineStage::TRANSFER, FPipelineStage::TRANSFER, FAccessFlag::TRANSFER_READ,
				                                  transferFamily, targetFamily);
			}

			ring.Buffers[slot]->End();

			const uint64 acquireSignalValue = ++s_UploadTimelineValue;
			SubmitDesc   acquireSubmit      = {};
			acquireSubmit.CommandBuffers    = {ring.Buffers[slot]};
			acquireSubmit.WaitSyncPoints    = {{s_pUploadSyncPoint.get(), transferSignalValue}};
			acquireSubmit.SignalSyncPoints  = {{s_pUploadSyncPoint.get(), acquireSignalValue}};
			RenderAPI::GetCommandQueue(target)->Submit(acquireSubmit);

			highestSignalValue = std::max(highestSignalValue, acquireSignalValue);

			for (const auto& handle : texturesByTarget[target])
				s_Textures[handle.GetIndex()].PendingUploadValue = acquireSignalValue;
			for (const auto& bufferHandle : buffersByTarget[target])
				s_Buffers[bufferHandle.GetIndex()].PendingUploadValue = acquireSignalValue;

			// Any ResizeBuffer copy targeting this queue is now retirable once that value is
			// reached
			for (const auto& copy : s_PendingBufferCopies)
				if (copy.TargetQueue == target)
					EnqueueBufferDestroy(copy.SrcHandle.GetIndex(), acquireSignalValue);
		}

		// Same-family uploads/copies never went through an acquire above, but a consumer on that
		// queue still needs to wait for the transfer submit itself to finish before it's visible.
		for (const auto& upload : s_PendingTextureUploads)
			if (RenderAPI::GetCommandQueue(upload.TargetQueue)->GetQueueFamilyIndex() == transferFamily)
				s_Textures[upload.Handle.GetIndex()].PendingUploadValue = transferSignalValue;
		for (const auto& upload : s_PendingBufferUploads)
			if (RenderAPI::GetCommandQueue(upload.TargetQueue)->GetQueueFamilyIndex() == transferFamily)
				s_Buffers[upload.Handle.GetIndex()].PendingUploadValue = transferSignalValue;
		for (const auto& copy : s_PendingBufferCopies)
			if (RenderAPI::GetCommandQueue(copy.TargetQueue)->GetQueueFamilyIndex() == transferFamily)
			{
				s_Buffers[copy.DstHandle.GetIndex()].PendingUploadValue = transferSignalValue;
				EnqueueBufferDestroy(copy.SrcHandle.GetIndex(), transferSignalValue);
			}

		s_SlotSignalValue[slot] = highestSignalValue;

		s_PendingTextureUploads.clear();
		s_PendingBufferUploads.clear();
		s_PendingBufferCopies.clear();
	}

	void ResourceManager::Update()
	{
		std::lock_guard<std::recursive_mutex> lock(s_Mutex);

		FlushUploads();

		s_CurrentFrame++;

		const auto isSafeToFree = [](const PendingDestroy& entry) {
			if (entry.RequiredSyncValue != 0)
				return s_pUploadSyncPoint->GetValue() >= entry.RequiredSyncValue;
			return s_CurrentFrame - entry.DestroyedOnFrame >= FRAMES_IN_FLIGHT;
		};

		std::erase_if(s_PendingTextureDestroys, [&isSafeToFree](const PendingDestroy& entry) {
			if (!isSafeToFree(entry))
				return false;

			TextureSlot& slot = s_Textures[entry.Index];
			slot.pTexture.reset();
			slot.pDefaultView.reset();
			slot.Generation++;
			s_FreeTextureIndices.push_back(entry.Index);
			return true;
		});

		std::erase_if(s_PendingBufferDestroys, [&isSafeToFree](const PendingDestroy& entry) {
			if (!isSafeToFree(entry))
				return false;

			BufferSlot& slot = s_Buffers[entry.Index];
			slot.pBuffer.reset();
			slot.Generation++;
			s_FreeBufferIndices.push_back(entry.Index);
			return true;
		});
	}

	std::vector<ResourceManager::TextureInfo> ResourceManager::GetAllTextures()
	{
		std::lock_guard<std::recursive_mutex> lock(s_Mutex);

		std::vector<TextureInfo> result;
		for (uint32 i = 0; i < s_Textures.size(); i++)
		{
			const TextureSlot& slot = s_Textures[i];
			if (!slot.Alive || !slot.pTexture) // skip dead slots and externally-registered (non-owned) ones
				continue;

			result.push_back({TextureHandle(i, slot.Generation), slot.Width, slot.Height, slot.Format, slot.DebugName});
		}
		return result;
	}

	std::vector<ResourceManager::BufferInfo> ResourceManager::GetAllBuffers()
	{
		std::lock_guard<std::recursive_mutex> lock(s_Mutex);

		std::vector<BufferInfo> result;
		for (uint32 i = 0; i < s_Buffers.size(); i++)
		{
			const BufferSlot& slot = s_Buffers[i];
			if (!slot.Alive)
				continue;

			result.push_back({BufferHandle(i, slot.Generation), slot.pBuffer->GetSize(), slot.DebugName});
		}
		return result;
	}

	const DescriptorSetLayout& ResourceManager::GetSetLayoutDesc()
	{
		return s_SetLayoutDesc;
	}

	DescriptorSet* ResourceManager::GetDescriptorSet()
	{
		return s_pHeapSet.get();
	}
} // namespace Poly
