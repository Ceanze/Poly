#pragma once

#include "Platform/Common/IPlatformRenderer.h"
#include "Platform/Vulkan/Renderers/TestRenderer.h"

namespace Poly
{

	class VulkanRenderer : public IPlatformRenderer
	{
	public:
		VulkanRenderer() = default;
		virtual ~VulkanRenderer() = default;

		virtual void init(uint32_t width, uint32_t height) override;
		virtual void beginScene() override;
		virtual void setActiveCamera(Camera* camera) override { this->camera = camera; }
		virtual void draw(Model * model) override; // More draws will be created in the future as overloads
		virtual void endScene() override;
		virtual void shutdown() override;

		virtual void createRenderer(/*Renderer::MESH*/) override {};
		virtual VertexBuffer* createVertexBuffer() override { return nullptr; };
		virtual IndexBuffer* createIndexBuffer() override { return nullptr; };
		virtual UniformBuffer* createUniformBuffer() override { return nullptr; };
		virtual StorageBuffer* createStorageBuffer() override { return nullptr; };
		virtual Texture* createTexture() override { return nullptr; };

	private:
		Camera* camera;
		Window* window;

		// Renderers
		TestRenderer testRenderer;
	};

}