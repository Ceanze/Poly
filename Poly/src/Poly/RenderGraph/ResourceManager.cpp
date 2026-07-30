#include "ResourceManager.h"

#include "Platform/API/CommandBuffer.h"
#include "Platform/API/CommandPool.h"
#include "Platform/API/CommandQueue.h"
#include "Platform/API/DescriptorSet.h"
#include "Poly/Core/RenderAPI.h"

#include <cstring>
#include <tuple>

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

		s_pTransferCommandPool   = RenderAPI::CreateCommandPool(FQueueType::TRANSFER, FCommandPoolFlags::NONE);
		s_pTransferCommandBuffer = s_pTransferCommandPool->AllocateCommandBuffer(ECommandBufferLevel::PRIMARY);

		s_pGraphicsCommandPool   = RenderAPI::CreateCommandPool(FQueueType::GRAPHICS, FCommandPoolFlags::NONE);
		s_pGraphicsCommandBuffer = s_pGraphicsCommandPool->AllocateCommandBuffer(ECommandBufferLevel::PRIMARY);

		s_DefaultLinearSampler  = GetOrCreateSampler(Sampler::GetDefaultLinearSampler()->GetDesc());
		s_DefaultNearestSampler = GetOrCreateSampler(Sampler::GetDefaultNearestSampler()->GetDesc());
	}

	void ResourceManager::Release()
	{
		RenderAPI::GetCommandQueue(FQueueType::TRANSFER)->Wait();
		RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Wait();

		s_pTransferCommandPool.reset();
		s_pGraphicsCommandPool.reset();

		s_Textures.clear();
		s_FreeTextureIndices.clear();
		s_Buffers.clear();
		s_FreeBufferIndices.clear();
		s_Samplers.clear();
		s_SamplerCache.clear();
		s_PendingTextureUploads.clear();
		s_PendingBufferUploads.clear();
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
		s_PendingTextureDestroys.emplace_back(handle.GetIndex(), s_CurrentFrame);
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
		s_PendingBufferDestroys.emplace_back(handle.GetIndex(), s_CurrentFrame);
	}

	void ResourceManager::UploadTextureData(TextureHandle handle, const void* pData, uint32 width, uint32 height)
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
		upload.Handle = handle;
		upload.Width  = width;
		upload.Height = height;
		upload.Data.assign(static_cast<const byte*>(pData), static_cast<const byte*>(pData) + static_cast<size_t>(width) * height * 4);
		s_PendingTextureUploads.push_back(std::move(upload));
	}

	void ResourceManager::UploadBufferData(BufferHandle handle, const void* pData, uint64 size, uint64 offset)
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
		upload.Handle = handle;
		upload.Offset = offset;
		upload.Data.assign(static_cast<const byte*>(pData), static_cast<const byte*>(pData) + size);
		s_PendingBufferUploads.push_back(std::move(upload));
	}

	void ResourceManager::FlushUploads()
	{
		if (s_PendingTextureUploads.empty() && s_PendingBufferUploads.empty())
			return;

		uint64 totalStagingSize = 0;
		for (const auto& upload : s_PendingTextureUploads)
			totalStagingSize += upload.Data.size();
		for (const auto& upload : s_PendingBufferUploads)
			totalStagingSize += upload.Data.size();

		BufferDesc stagingDesc  = {};
		stagingDesc.BufferUsage = FBufferUsage::TRANSFER_SRC;
		stagingDesc.MemUsage    = EMemoryUsage::CPU_VISIBLE;
		stagingDesc.Size        = totalStagingSize;
		Ref<Buffer> pStaging    = RenderAPI::CreateBuffer(&stagingDesc);

		void*  pMapped = pStaging->Map();
		uint64 offset  = 0;

		const uint32 transferFamily = RenderAPI::GetCommandQueue(FQueueType::TRANSFER)->GetQueueFamilyIndex();
		const uint32 graphicsFamily = RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->GetQueueFamilyIndex();
		const bool   sameQueue      = transferFamily == graphicsFamily;

		s_pTransferCommandPool->Reset();
		s_pTransferCommandBuffer->Begin(FCommandBufferFlag::ONE_TIME_SUBMIT);

		for (const auto& upload : s_PendingTextureUploads)
		{
			memcpy(static_cast<byte*>(pMapped) + offset, upload.Data.data(), upload.Data.size());
			Texture* pTexture = s_Textures[upload.Handle.GetIndex()].pTexture.get();

			s_pTransferCommandBuffer->PipelineTextureBarrier(pTexture, FPipelineStage::ALL_COMMANDS, FPipelineStage::TRANSFER, FAccessFlag::NONE,
			                                                 FAccessFlag::TRANSFER_WRITE, ETextureLayout::UNDEFINED, ETextureLayout::TRANSFER_DST_OPTIMAL);

			CopyBufferDesc copyDesc = {};
			copyDesc.BufferOffset   = offset;
			copyDesc.Width          = upload.Width;
			copyDesc.Height         = upload.Height;
			copyDesc.Depth          = 1;
			copyDesc.ArrayCount     = 1;
			s_pTransferCommandBuffer->CopyBufferToTexture(pStaging.get(), pTexture, ETextureLayout::TRANSFER_DST_OPTIMAL, copyDesc);

			s_pTransferCommandBuffer->ReleaseTexture(pTexture, FPipelineStage::TRANSFER, FPipelineStage::TRANSFER, FAccessFlag::TRANSFER_READ,
			                                         ETextureLayout::TRANSFER_DST_OPTIMAL, ETextureLayout::SHADER_READ_ONLY_OPTIMAL, transferFamily,
			                                         graphicsFamily);

			offset += upload.Data.size();
		}

		for (const auto& upload : s_PendingBufferUploads)
		{
			memcpy(static_cast<byte*>(pMapped) + offset, upload.Data.data(), upload.Data.size());
			Buffer* pBuffer = s_Buffers[upload.Handle.GetIndex()].pBuffer.get();

			s_pTransferCommandBuffer->CopyBuffer(pStaging.get(), pBuffer, upload.Data.size(), offset, upload.Offset);
			s_pTransferCommandBuffer->ReleaseBuffer(pBuffer, FPipelineStage::TRANSFER, FPipelineStage::TRANSFER, FAccessFlag::TRANSFER_READ,
			                                        transferFamily, graphicsFamily);

			offset += upload.Data.size();
		}

		pStaging->Unmap();
		s_pTransferCommandBuffer->End();

		SubmitDesc transferSubmit     = {};
		transferSubmit.CommandBuffers = {s_pTransferCommandBuffer};
		RenderAPI::GetCommandQueue(FQueueType::TRANSFER)->SubmitIdle(transferSubmit);

		if (!sameQueue)
		{
			s_pGraphicsCommandPool->Reset();
			s_pGraphicsCommandBuffer->Begin(FCommandBufferFlag::ONE_TIME_SUBMIT);

			for (const auto& upload : s_PendingTextureUploads)
			{
				Texture* pTexture = s_Textures[upload.Handle.GetIndex()].pTexture.get();
				s_pGraphicsCommandBuffer->AcquireTexture(pTexture, FPipelineStage::TRANSFER, FPipelineStage::TRANSFER, FAccessFlag::TRANSFER_READ,
				                                         ETextureLayout::TRANSFER_DST_OPTIMAL, ETextureLayout::SHADER_READ_ONLY_OPTIMAL, transferFamily,
				                                         graphicsFamily);
			}
			for (const auto& upload : s_PendingBufferUploads)
			{
				Buffer* pBuffer = s_Buffers[upload.Handle.GetIndex()].pBuffer.get();
				s_pGraphicsCommandBuffer->AcquireBuffer(pBuffer, FPipelineStage::TRANSFER, FPipelineStage::TRANSFER, FAccessFlag::TRANSFER_READ,
				                                        transferFamily, graphicsFamily);
			}

			s_pGraphicsCommandBuffer->End();

			SubmitDesc graphicsSubmit     = {};
			graphicsSubmit.CommandBuffers = {s_pGraphicsCommandBuffer};
			RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->SubmitIdle(graphicsSubmit);
		}

		s_PendingTextureUploads.clear();
		s_PendingBufferUploads.clear();
	}

	void ResourceManager::Update()
	{
		std::lock_guard<std::recursive_mutex> lock(s_Mutex);

		FlushUploads();

		s_CurrentFrame++;

		std::erase_if(s_PendingTextureDestroys, [](const std::pair<uint32, uint64>& entry) {
			if (s_CurrentFrame - entry.second < FRAMES_IN_FLIGHT)
				return false;

			TextureSlot& slot = s_Textures[entry.first];
			slot.pTexture.reset();
			slot.pDefaultView.reset();
			slot.Generation++;
			s_FreeTextureIndices.push_back(entry.first);
			return true;
		});

		std::erase_if(s_PendingBufferDestroys, [](const std::pair<uint32, uint64>& entry) {
			if (s_CurrentFrame - entry.second < FRAMES_IN_FLIGHT)
				return false;

			BufferSlot& slot = s_Buffers[entry.first];
			slot.pBuffer.reset();
			slot.Generation++;
			s_FreeBufferIndices.push_back(entry.first);
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
