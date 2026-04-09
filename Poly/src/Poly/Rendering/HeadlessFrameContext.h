#pragma once

#include "FrameContext.h"

namespace Poly
{
	class SyncPoint;
	class CommandQueue;

	struct HeadlessFrameContextDesc
	{
		CommandQueue* pQueue      = nullptr;
		uint32        Width       = 0;
		uint32        Height      = 0;
		uint32        BufferCount = 0;
		EFormat       Format      = EFormat::B8G8R8A8_UNORM;
	};

	class HeadlessFrameContext : public FrameContext
	{
	public:
		CLASS_REMOVE_COPY(HeadlessFrameContext);
		CLASS_REMOVE_MOVE(HeadlessFrameContext);

		explicit HeadlessFrameContext(const HeadlessFrameContextDesc& desc);
		~HeadlessFrameContext();

		PresentResult    Present(const std::vector<CommandBuffer*>& commandBuffers) override;
		Ref<Texture>     GetTexture(uint32 bufferIndex) const override;
		Ref<TextureView> GetTextureView(uint32 bufferIndex) const override;
		uint32           GetBackbufferIndex() const override;
		uint32           GetBackbufferCount() const override;
		void             OnWindowResized(int width, int height) override {}
		PolyID           GetID() const override;
		uint32           GetWidth() const override;
		uint32           GetHeight() const override;

	private:
		PolyID                        m_ID;
		uint32                        m_Width          = 0;
		uint32                        m_Height         = 0;
		uint32                        m_BufferCount    = 0;
		uint32                        m_CurrentIndex   = 0;
		uint64                        m_FrameSyncValue = 0;
		CommandQueue*                 m_pQueue         = nullptr;
		std::vector<Ref<Texture>>     m_Textures;
		std::vector<Ref<TextureView>> m_TextureViews;
		Ref<SyncPoint>                m_pFrameSyncPoint;
	};
} // namespace Poly
