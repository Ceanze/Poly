#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Poly/Core/Renderer.h"
#include "Poly/Core/Window.h"
#include "PVKInstance.h"
#include "PVKSwapChain.h"
#include "PVKPipeline.h"
#include "PVKShader.h"
#include "PVKRenderPass.h"
#include "PVKFramebuffer.h"
#include "PVKCommandPool.h"


namespace Poly
{

	class VulkanRenderer : public Renderer
	{
	public:
		VulkanRenderer() = default;
		virtual ~VulkanRenderer() = default;
		virtual void initialize(unsigned width = 1280, unsigned height = 720);
		virtual void setWinTitle(const char* title);
		//virtual void present() = 0;
		virtual void shutdown();

		virtual void setClearColor(float r, float g, float b, float a);
		virtual void clearBuffer(unsigned int);

	private:
		PVKInstance instance;
		PVKSwapChain swapChain;
		PVKPipeline pipeline;
		PVKShader shader;
		PVKRenderPass renderPass;
		std::vector<PVKFramebuffer> framebuffers;
		PVKCommandPool commandPool;

		Window* window;
	};

}