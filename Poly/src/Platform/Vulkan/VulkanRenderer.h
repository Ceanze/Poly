#pragma once

#include "Platform/Common/IPlatformRenderer.h"
#include "Platform/Vulkan/PVKSwapChain.h"
#include "Platform/Vulkan/PVKTypes.h"

namespace Poly
{
	class PVKCommandBuffer;
	class IRenderer;

	class VulkanRenderer : public IPlatformRenderer
	{
	public:
		VulkanRenderer() = default;
		virtual ~VulkanRenderer() = default;

		virtual void init(uint32_t width, uint32_t height) override;
		virtual void beginScene() override;
		virtual void draw(Model * model) override; // More draws will be created in the future as overloads
		virtual void endScene() override;
		virtual void shutdown() override;

		virtual void createRenderer(Renderer subRenderer) override;
		virtual VertexBuffer* createVertexBuffer() override { return nullptr; };
		virtual IndexBuffer* createIndexBuffer() override { return nullptr; };
		virtual UniformBuffer* createUniformBuffer() override { return nullptr; };
		virtual StorageBuffer* createStorageBuffer() override { return nullptr; };
		virtual Texture* createTexture() override { return nullptr; };

		// Adds command buffer to be submitted for this frame
		void addCommandBuffer(QueueType queueType, PVKCommandBuffer* buffer);

		PVKSwapChain* getSwapChain() { return &this->swapChain; }

	private:
		void createSyncObjects();

		Window* window = nullptr;

		// General vulkan
		PVKSwapChain swapChain;
		std::vector<PVKCommandBuffer*> graphicsBuffers; // Only for current frame
		uint32_t imageIndex = 0;

		// Sync
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;
		size_t currentFrame = 0;
		const int MAX_FRAMES_IN_FLIGHT = 2;

		// Renderers
		std::vector<IRenderer*> subRenderers;
	};

}