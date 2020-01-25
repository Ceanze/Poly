#pragma once
#include "Poly/Core/Renderer.h"
#include "Poly/Core/Window.h"
#include "PVKInstance.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Poly
{

	class VulkanRenderer : public Renderer
	{
	public:
		VulkanRenderer() = default;
		virtual void initialize(unsigned width = 800, unsigned height = 600);
		virtual void setWinTitle(const char* title);
		//virtual void present() = 0;
		virtual void shutdown();

		virtual void setClearColor(float r, float g, float b, float a);
		virtual void clearBuffer(unsigned int);

	private:
		PVKInstance* instance;

		Window* window;
	};

}