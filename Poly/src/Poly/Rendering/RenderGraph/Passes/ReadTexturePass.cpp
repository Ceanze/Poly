#include "ReadTexturePass.h"

#include "Platform/API/Buffer.h"
#include "Platform/API/CommandBuffer.h"
#include "Platform/API/SyncPoint.h"
#include "Platform/API/Texture.h"
#include "Poly/Core/RenderAPI.h"
#include "Poly/Rendering/RenderGraph/RenderContext.h"
#include "Poly/Rendering/RenderGraph/RenderData.h"
#include "Poly/Rendering/RenderGraph/Resource.h"

namespace Poly
{
	static uint32 GetBytesPerPixel(EFormat format)
	{
		switch (format)
		{
		case EFormat::R8G8B8A8_UNORM:
		case EFormat::B8G8R8A8_UNORM:
		case EFormat::D24_UNORM_S8_UINT:
		case EFormat::R32_SFLOAT:
			return 4;
		default:
			POLY_CORE_WARN("ReadTexturePass::GetBytesPerPixel - unknown format, defaulting to 4 bytes per pixel");
			return 4;
		}
	}

	ReadTexturePass::ReadTexturePass()
	{
		p_Type      = Pass::Type::SYNC; // TODO: Update to TRANSFER
		m_SyncPoint = RenderAPI::CreateSyncPoint();
	}

	Ref<ReadTexturePass> ReadTexturePass::Create()
	{
		return CreateRef<ReadTexturePass>();
	}

	PassReflection ReadTexturePass::Reflect()
	{
		PassReflection reflection;

		reflection.AddPassthrough("InputTexture").TextureLayout(ETextureLayout::TRANSFER_SRC_OPTIMAL).BindPoint(FResourceBindPoint::COPY_SRC);

		return reflection;
	}

	void ReadTexturePass::Execute(RenderContext& context, const RenderData& renderData)
	{
		const Resource* pResource = renderData["InputTexture"];
		if (!pResource)
		{
			POLY_CORE_ERROR("ReadTexturePass - InputTexture resource not found");
			return;
		}

		if (!pResource->IsTexture())
		{
			POLY_CORE_ERROR("ReadTexturePass - InputTexture resource is not a texture");
			return;
		}

		m_DeadStagingBuffers[context.GetImageIndex()].clear();

		const Texture* pTexture = pResource->GetAsTexture();
		uint32         width    = pTexture->GetWidth();
		uint32         height   = pTexture->GetHeight();
		uint32         bpp      = GetBytesPerPixel(pTexture->GetDesc().Format);
		uint64         dataSize = static_cast<uint64>(width) * height * bpp;

		if (m_StagingBuffers.empty() || m_StagingWidth != width || m_StagingHeight != height)
		{
			AllocateStagingBuffers(width, height, pTexture->GetDesc().Format, 2, context.GetImageIndex());
			m_StagingWidth  = width;
			m_StagingHeight = height;
		}

		CopyBufferDesc copyDesc = {};
		copyDesc.Width          = width;
		copyDesc.Height         = height;
		copyDesc.Depth          = 1;
		copyDesc.ArrayLayer     = 0;
		copyDesc.ArrayCount     = 1;
		copyDesc.MipLevel       = 0;

		CommandBuffer* pCmd = context.GetCommandBuffer();
		pCmd->CopyTextureToBuffer(pTexture, m_StagingBuffers[context.GetImageIndex()].get(), ETextureLayout::TRANSFER_SRC_OPTIMAL, copyDesc);

		pCmd->PipelineBufferBarrier(
		    m_StagingBuffers[context.GetImageIndex()].get(),
		    FPipelineStage::TRANSFER,
		    FPipelineStage::HOST,
		    FAccessFlag::TRANSFER_WRITE,
		    FAccessFlag::HOST_READ);

		context.AddSignalSyncPoint({m_SyncPoint.get(), ++m_SyncID});
	}

	void ReadTexturePass::CopyData(void* pDst, uint64 size) const
	{
		// RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Wait(); // Ensure GPU has finished writing to the staging buffer
		if (m_StagingBuffers.empty())
		{
			POLY_CORE_ERROR("ReadTexturePass::CopyData - staging buffers not yet allocated, execute the pass first");
			return;
		}

		uint64 currentIndex = (m_SyncPoint->GetValue() - 1) % m_StagingBuffers.size();
		if (currentIndex >= m_StagingBuffers.size())
		{
			POLY_CORE_ERROR("ReadTexturePass::CopyData - invalid staging buffer index");
			return;
		}

		void* pMapped = m_StagingBuffers[currentIndex]->Map();
		memcpy(pDst, pMapped, size);
		m_StagingBuffers[currentIndex]->Unmap();
	}

	uint64 ReadTexturePass::GetDataSize() const
	{
		if (m_StagingBuffers.empty())
			return 0;

		return m_StagingBuffers.front()->GetSize();
	}

	void ReadTexturePass::AllocateStagingBuffers(uint32 width, uint32 height, EFormat format, uint32 count, uint32 imageIndex)
	{
		for (auto& buffer : m_StagingBuffers)
		{
			m_DeadStagingBuffers[imageIndex].push_back(buffer);
		}
		m_StagingBuffers.clear();

		for (uint32 i = 0; i < count; ++i)
		{
			uint32 bytesPerPixel = GetBytesPerPixel(format);
			uint64 bufferSize    = static_cast<uint64>(width) * height * bytesPerPixel;

			BufferDesc desc  = {};
			desc.Size        = bufferSize;
			desc.MemUsage    = EMemoryUsage::CPU_VISIBLE;
			desc.BufferUsage = FBufferUsage::TRANSFER_DST;

			m_StagingBuffers.push_back(RenderAPI::CreateBuffer(&desc));
		}
	}
} // namespace Poly
