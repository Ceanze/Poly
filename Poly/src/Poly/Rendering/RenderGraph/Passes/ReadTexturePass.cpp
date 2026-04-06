#include "ReadTexturePass.h"

#include "Platform/API/Buffer.h"
#include "Platform/API/CommandBuffer.h"
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
		p_Type = Pass::Type::SYNC; // TODO: Update to TRANSFER
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

	void ReadTexturePass::Execute(const RenderContext& context, const RenderData& renderData)
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

		const Texture* pTexture = pResource->GetAsTexture();
		uint32         width    = pTexture->GetWidth();
		uint32         height   = pTexture->GetHeight();
		uint32         bpp      = GetBytesPerPixel(pTexture->GetDesc().Format);
		uint64         dataSize = static_cast<uint64>(width) * height * bpp;

		if (!m_pStagingBuffer || m_StagingWidth != width || m_StagingHeight != height)
		{
			BufferDesc desc  = {};
			desc.Size        = dataSize;
			desc.MemUsage    = EMemoryUsage::CPU_VISIBLE;
			desc.BufferUsage = FBufferUsage::TRANSFER_DST;

			m_pStagingBuffer = RenderAPI::CreateBuffer(&desc);
			m_StagingWidth   = width;
			m_StagingHeight  = height;
		}

		CopyBufferDesc copyDesc = {};
		copyDesc.Width          = width;
		copyDesc.Height         = height;
		copyDesc.Depth          = 1;
		copyDesc.ArrayLayer     = 0;
		copyDesc.ArrayCount     = 1;
		copyDesc.MipLevel       = 0;

		CommandBuffer* pCmd = context.GetCommandBuffer();
		pCmd->CopyTextureToBuffer(pTexture, m_pStagingBuffer.get(), ETextureLayout::TRANSFER_SRC_OPTIMAL, copyDesc);

		pCmd->PipelineBufferBarrier(
		    m_pStagingBuffer.get(),
		    FPipelineStage::TRANSFER,
		    FPipelineStage::HOST,
		    FAccessFlag::TRANSFER_WRITE,
		    FAccessFlag::HOST_READ);
	}

	void ReadTexturePass::CopyData(void* pDst, uint64 size) const
	{
		if (!m_pStagingBuffer)
		{
			POLY_CORE_ERROR("ReadTexturePass::CopyData - staging buffer not yet allocated, execute the pass first");
			return;
		}

		void* pMapped = m_pStagingBuffer->Map();
		memcpy(pDst, pMapped, size);
		m_pStagingBuffer->Unmap();
	}

	uint64 ReadTexturePass::GetDataSize() const
	{
		if (!m_pStagingBuffer)
			return 0;
		return m_pStagingBuffer->GetSize();
	}
} // namespace Poly
