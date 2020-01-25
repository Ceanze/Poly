#include "polypch.h"
#include "Renderer.h"
#include "Platform/Vulkan/VulkanRenderer.h"

namespace Poly
{

	Renderer* Renderer::create(BACKEND backend)
	{
		switch (backend)
		{
		case BACKEND::VULKAN:
			return new VulkanRenderer();
		default:
			POLY_CORE_FATAL("Only Vulkan renderer is currently supported!");
		}
	}

}