#include "polypch.h"
#include "Sampler.h"

#include "Poly/Core/RendererAPI.h"

namespace Poly
{

	Ref<Sampler> Sampler::create()
	{
		switch (RendererAPI::getAPI())
		{
		case BackendAPI::NONE: POLY_ASSERT(false, "No renderer API is selected!")
			//case BackendAPI::VULKAN: return createRef<PVKSampler>();
		}
		return Ref<Sampler>();
	}

	Ref<Sampler> Sampler::create(Filter filter, AddressMode addressMode, Filter mipmap)
	{
		switch (RendererAPI::getAPI())
		{
		case BackendAPI::NONE: POLY_ASSERT(false, "No renderer API is selected!")
			//case BackendAPI::VULKAN: return createRef<PVKSampler>();
		}
		return Ref<Sampler>();
	}

}