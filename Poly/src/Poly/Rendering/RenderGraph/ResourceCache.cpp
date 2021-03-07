#include "polypch.h"
#include "ResourceCache.h"

#include "Poly/Core/RenderAPI.h"
#include "Platform/API/Buffer.h"
#include "Platform/API/Texture.h"
#include "Platform/API/CommandQueue.h"
#include "Resource.h"

namespace Poly
{
	Ref<ResourceCache> ResourceCache::Create()
	{
		return CreateRef<ResourceCache>();
	}
	
	void ResourceCache::RegisterExternalResource(const std::string& name, Ref<Resource> pResource)
	{
		if (m_NameToExternalIndex.contains(name))
			m_ExternalResources[m_NameToExternalIndex[name]] = pResource;
		else
		{
			m_NameToExternalIndex[name] = m_ExternalResources.size();
			m_ExternalResources.push_back(pResource);
		}
	}

	void ResourceCache::RegisterResource(const std::string& name, uint32 timepoint, IOData iodata, const std::string& alias)
	{
		if (m_NameToIndex.contains(name))
		{
			POLY_CORE_WARN("Resource {} has already been added, ignoring call", name);
			return;
		}

		bool isAlias = !alias.empty();
		if (isAlias && (!m_NameToIndex.contains(alias) && !m_NameToExternalIndex.contains(alias)))
		{
			POLY_CORE_WARN("Resource {} cannot use alias {}, alias resource has not been registered", name, alias);
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
			m_Resources.push_back(data);
		}
		else // Aliased resource
		{
			if (m_NameToIndex.contains(alias))
			{
				uint32 index = m_NameToIndex[alias];
				m_NameToIndex[name] = index;
				CalcLifetime(m_Resources[index].Lifetime, timepoint);
			}
			else
			{
				m_NameToExternalIndex[name] = m_NameToExternalIndex[alias];
			}
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

				resourceData.pResource = Resource::Create(RenderAPI::CreateBuffer(&desc), resourceData.Name);
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
				desc.Format			= resourceData.IOInfo.Format;

				resourceData.pResource = Resource::Create(RenderAPI::CreateTexture(&desc), resourceData.Name);
			}
		}
	}

	Ref<Resource> ResourceCache::GetResource(const std::string& name)
	{
		if (!m_NameToIndex.contains(name) && !m_NameToExternalIndex.contains(name))
		{
			POLY_CORE_WARN("Resource {} cannot be gotten, it does not exist", name);
			return nullptr;
		}

		if (m_NameToIndex.contains(name))
			return m_Resources[m_NameToIndex[name]].pResource;
		else
			return m_ExternalResources[m_NameToExternalIndex[name]];
	}

	void ResourceCache::Reset()
	{
		m_NameToIndex.clear();
		m_NameToExternalIndex.clear();

		// Wait for GPU since graphic objects might be deleted
		RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Wait();
		m_Resources.clear();
		m_ExternalResources.clear();
	}

	void ResourceCache::CalcLifetime(std::pair<uint32, uint32>& lifetime, uint32 newTimepoint)
	{
		if (newTimepoint <= lifetime.first)
			lifetime.first = newTimepoint;
		else if (newTimepoint > lifetime.second)
			lifetime.second = newTimepoint;
	}
}