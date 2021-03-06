#pragma once

#include "Platform/Common/IPlatformRenderer.h"
#include "Platform/Vulkan/PVKSwapChain.h"
#include "Platform/Vulkan/PVKTypes.h"

namespace Poly
{
	class CommandBuffer;
	class IRenderer;

	class VulkanRenderer : public IPlatformRenderer
	{
	public:
		VulkanRenderer() = default;
		virtual ~VulkanRenderer();

		virtual void Init(uint32_t width, uint32_t height) override;
		virtual void BeginScene() override;
		virtual void Draw(Model * model) override; // More draws will be created in the future as overloads
		virtual void EndScene() override;

		virtual void CreateRenderer(Renderer subRenderer) override;
		virtual VertexBuffer* CreateVertexBuffer() override { return nullptr; };
		virtual IndexBuffer* CreateIndexBuffer() override { return nullptr; };
		virtual UniformBuffer* CreateUniformBuffer() override { return nullptr; };
		virtual StorageBuffer* CreateStorageBuffer() override { return nullptr; };
		virtual Texture* CreateTexture() override { return nullptr; };

		// Adds command buffer to be submitted for this frame
		void AddCommandBuffer(FQueueType queueType, CommandBuffer* pBuffer);

		SwapChain* GetSwapChain() { return m_SwapChain.get(); }

	private:
		Window* m_pWindow = nullptr;

		// General vulkan
		Ref<SwapChain> m_SwapChain;
		std::vector<CommandBuffer*> m_GraphicsBuffers;

		// Renderers
		std::vector<IRenderer*> m_SubRenderers;
	};

}