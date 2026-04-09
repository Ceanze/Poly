#include "HeadlessFrameContext.h"

#include "Platform/API/CommandQueue.h"
#include "Platform/API/SyncPoint.h"
#include "Platform/API/Texture.h"
#include "Platform/API/TextureView.h"
#include "Poly/Core/RenderAPI.h"
#include "polypch.h"

namespace Poly
{
	HeadlessFrameContext::HeadlessFrameContext(const HeadlessFrameContextDesc& desc)
	    : m_Width(desc.Width)
	    , m_Height(desc.Height)
	    , m_BufferCount(desc.BufferCount)
	    , m_pQueue(desc.pQueue)
	{
		m_pFrameSyncPoint = RenderAPI::CreateSyncPoint();

		for (uint32 i = 0; i < m_BufferCount; i++)
		{
			std::string debugName = "HeadlessBackbuffer " + std::to_string(i);

			TextureDesc texDesc = {
			    .Width        = m_Width,
			    .Height       = m_Height,
			    .Depth        = 1,
			    .ArrayLayers  = 1,
			    .MipLevels    = 1,
			    .SampleCount  = 1,
			    .MemoryUsage  = EMemoryUsage::GPU_ONLY,
			    .Format       = desc.Format,
			    .TextureUsage = FTextureUsage::COLOR_ATTACHMENT | FTextureUsage::COPY_SRC,
			    .TextureDim   = ETextureDim::DIM_2D,
			    .DebugName    = debugName};
			Ref<Texture> pTexture = RenderAPI::CreateTexture(&texDesc);

			TextureViewDesc viewDesc = {
			    .pTexture        = pTexture.get(),
			    .ImageViewType   = EImageViewType::TYPE_2D,
			    .Format          = desc.Format,
			    .ImageViewFlag   = FImageViewFlag::COLOR,
			    .MipLevel        = 0,
			    .MipLevelCount   = 1,
			    .ArrayLayer      = 0,
			    .ArrayLayerCount = 1,
			    .DebugName       = debugName};
			Ref<TextureView> pView = RenderAPI::CreateTextureView(&viewDesc);

			m_Textures.push_back(pTexture);
			m_TextureViews.push_back(pView);
		}
	}

	HeadlessFrameContext::~HeadlessFrameContext() = default;

	PresentResult HeadlessFrameContext::Present(const std::vector<CommandBuffer*>& commandBuffers)
	{
		SubmitDesc submitDesc;
		submitDesc.CommandBuffers   = commandBuffers;
		submitDesc.SignalSyncPoints = {{m_pFrameSyncPoint.get(), ++m_FrameSyncValue}};
		m_pQueue->Submit(submitDesc);

		m_CurrentIndex = (m_CurrentIndex + 1) % m_BufferCount;

		const uint64 waitValue = (m_FrameSyncValue + 1) >= m_BufferCount ? (m_FrameSyncValue + 1) - m_BufferCount : 0;
		if (waitValue > 0)
			m_pFrameSyncPoint->Wait(waitValue);

		return PresentResult::SUCCESS;
	}

	Ref<Texture> HeadlessFrameContext::GetTexture(uint32 bufferIndex) const
	{
		return m_Textures[bufferIndex];
	}

	Ref<TextureView> HeadlessFrameContext::GetTextureView(uint32 bufferIndex) const
	{
		return m_TextureViews[bufferIndex];
	}

	uint32 HeadlessFrameContext::GetBackbufferIndex() const
	{
		return m_CurrentIndex;
	}

	uint32 HeadlessFrameContext::GetBackbufferCount() const
	{
		return m_BufferCount;
	}

	PolyID HeadlessFrameContext::GetID() const
	{
		return m_ID;
	}

	uint32 HeadlessFrameContext::GetWidth() const
	{
		return m_Width;
	}

	uint32 HeadlessFrameContext::GetHeight() const
	{
		return m_Height;
	}
} // namespace Poly
