#include "polypch.h"
#include "VulkanRenderer.h"

#include "Poly/Core/Window.h"

namespace Poly
{

	void VulkanRenderer::init(uint32_t width, uint32_t height)
	{
		// Call init on the sub renderers and create window
		this->window = new Window(width, height, "Vulkan renderer window");

		// Test renderer
		this->testRenderer.setWindow(this->window);
		this->testRenderer.setActiveCamera(this->camera);
		this->testRenderer.init();
	}

	void VulkanRenderer::beginScene()
	{
		// Just call the testRenderer - This should handle the sync and image getting later
		this->testRenderer.beginScene();
	}

	void VulkanRenderer::draw(Model* model)
	{
		// Call the subRenderers record function
		this->testRenderer.record();
	}

	void VulkanRenderer::endScene()
	{
		// Just call the testRenderer - This should handle the sync and iamge present later
		this->testRenderer.endScene();
	}

	void VulkanRenderer::shutdown()
	{
		// Cleanup all the sub renderer and own sync objects
		this->testRenderer.shutdown();
		delete this->window;
	}

}