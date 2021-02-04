#pragma once

#include "VulkanCommon.h"
#include "Platform/API/SwapChain.h"

namespace Poly
{
	class PVKFence;
	class PVKTexture;
	class PVKSemaphore;
	class PVKTextureView;

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	/*
		PVKSwapChain contains the swapchain and the surface with its creation
	*/
	class PVKSwapChain : public SwapChain
	{
	public:

		PVKSwapChain() = default;
		~PVKSwapChain();

		virtual void Init(const SwapChainDesc* pDesc) override final;

		virtual void Resize(uint32 width, uint32 height) override final;
		virtual void Present(std::vector<CommandBuffer*> commandBuffers, Semaphore* pWaitSemaphore) override final;

		uint64			GetNative() const { return reinterpret_cast<uint64>(m_SwapChain); }
		VkSwapchainKHR	GetNativeVK() const { return m_SwapChain; }
		VkFormat		GetFormatVK() const { return m_FormatVK; }
		VkExtent2D		GetExtentVK() const { return m_Extent; }
		virtual uint32	GetBackbufferIndex() const override final { return m_ImageIndex; }
		virtual uint32	GetBackbufferCount() const override final { return p_SwapchainDesc.BufferCount; }
		virtual Texture* GetTexture(uint32 bufferIndex) const override final { return reinterpret_cast<Texture*>(m_Textures.at(bufferIndex)); }
		const PVKTexture* GetTextureVK(uint32 bufferIndex) const { return m_Textures.at(bufferIndex); }
		virtual TextureView* GetTextureView(uint32 bufferIndex) const override final { return reinterpret_cast<TextureView*>(m_TextureViews.at(bufferIndex)); }
		const PVKTextureView* GetTextureViewVK(uint32 bufferIndex) const { return m_TextureViews.at(bufferIndex); }


	private:
		void CreateSwapChain();
		SwapChainSupportDetails QuerySwapChainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		void CreateImageViews();
		void CreateSyncObjects();
		void AcquireNextImage();

		VkSwapchainKHR					m_SwapChain		= VK_NULL_HANDLE;
		VkFormat						m_FormatVK		= VK_FORMAT_UNDEFINED;
		VkExtent2D						m_Extent		= {0, 0};
		uint32							m_ImageIndex	= 0;
		uint32							m_FrameIndex	= 0;
		std::vector<PVKTexture*>		m_Textures;
		std::vector<PVKTextureView*>	m_TextureViews;

		// Sync
		std::vector<PVKSemaphore*>		m_RenderSemaphores;
		std::vector<PVKSemaphore*>		m_AcquireSemaphores;
		std::vector<PVKFence*>			m_ImagesInFlight;
	};
}