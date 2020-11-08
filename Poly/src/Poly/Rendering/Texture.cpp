#include "polypch.h"
#include "Texture.h"

#include "Poly/Core/RendererAPI.h"
#include "Platform/Vulkan/PVKTexture.h"

namespace Poly
{

	Ref<Texture2D> Texture2D::create(uint32_t width, uint32_t height)
	{
		switch (RendererAPI::getAPI())
		{
		case BackendAPI::NONE: POLY_ASSERT(false, "No renderer API is selected!")
		//case BackendAPI::VULKAN: return createRef<PVKTexture>(); CONTINUE THE TEXTURE ABSTRACTATION (PVKTexture inherit from Texture2D)
		}
		return Ref<Texture2D>();
	}

	Ref<Texture2D> Texture2D::create(std::string path)
	{
		return Ref<Texture2D>();
	}

}