#include "polypch.h"
#include "ResourceCache.h"

#include "Poly/Core/RenderAPI.h"
#include "Platform/API/Buffer.h"
#include "Platform/API/Texture.h"
#include "Resource.h"

namespace Poly
{
	Ref<ResourceCache> ResourceCache::Create()
	{
		return CreateRef<ResourceCache>();
	}
	
	void ResourceCache::RegisterResource(const std::string& name, uint32 timepoint, IOData iodata, const std::string& alias)
	{
		if (m_NameToIndex.contains(name))
		{
			POLY_CORE_WARN("Resource {} has already been added, ignoring call", name);
			return;
		}

		bool isAlias = !alias.empty();
		if (isAlias && !m_NameToIndex.contains(alias))
		{
			POLY_CORE_WARN("Resource {} cannot use alias {}, alias has not been added", name, alias);
			return;
		}

		if (!isAlias) // New resource
		{
			uint32 index = m_Resources.size();
			m_NameToIndex[name] = index;
			ResourceData data = {};
			data.Lifetime	= {timepoint, timepoint};
			data.Name		= name;
			data.IOInfo		= iodata;
		}
		else // Aliased resource
		{
			uint32 index = m_NameToIndex[alias];
			m_NameToIndex[name] = index;
			CalcLifetime(m_Resources[index].Lifetime, timepoint);
		}
	}

	void ResourceCache::AllocateResources()
	{
		for (auto& resourceData : m_Resources)
		{
			FResourceBindPoint bindPoint = resourceData.IOInfo.BindPoint;

			if (bindPoint == FResourceBindPoint::STORAGE || bindPoint == FResourceBindPoint::UNIFORM)
			{
				BufferDesc desc = {};
				desc.BufferUsage	= bindPoint == FResourceBindPoint::STORAGE ? FBufferUsage::STORAGE_BUFFER : FBufferUsage::UNIFORM_BUFFER;
				desc.MemUsage		= EMemoryUsage::GPU_ONLY; // TODO: Check if staging buffers should/can be created here
				desc.Size			= resourceData.IOInfo.Size;

				resourceData.pResource = Resource::Create(RenderAPI::CreateBuffer(&desc));
			}
			else if (bindPoint == FResourceBindPoint::COLOR_ATTACHMENT || bindPoint == FResourceBindPoint::DEPTH_STENCIL)
			{
				TextureDesc desc = {};
				desc.Width			= resourceData.IOInfo.Width;
				desc.Height			= resourceData.IOInfo.Height;
				desc.Depth			= 1;
				desc.ArrayLayers	= 1;
				desc.MipLevels		= 1; // TODO: Add support for mips
				desc.SampleCount	= 1;
				desc.TextureDim		= ETextureDim::DIM_2D;
				desc.TextureUsage	= bindPoint == FResourceBindPoint::COLOR_ATTACHMENT ? FTextureUsage::COLOR_ATTACHMENT : FTextureUsage::DEPTH_STENCIL_ATTACHMENT;

				resourceData.pResource = Resource::Create(RenderAPI::CreateTexture(&desc));
			}
		}
	}

	Ref<Resource> ResourceCache::GetResource(const std::string& name)
	{
		if (!m_NameToIndex.contains(name))
		{
			POLY_CORE_WARN("Resource {} cannot be gotten, it does not exist", name);
			return nullptr;
		}

		return m_Resources[m_NameToIndex[name]].pResource;
	}

	void ResourceCache::CalcLifetime(std::pair<uint32, uint32>& lifetime, uint32 newTimepoint)
	{
		if (newTimepoint <= lifetime.first)
			lifetime.first = newTimepoint;
		else if (newTimepoint > lifetime.second)
			lifetime.second = newTimepoint;
	}

	FBufferUsage ResourceCache::GetBufferUsage(EResourceType resourceType)
	{

	}
}