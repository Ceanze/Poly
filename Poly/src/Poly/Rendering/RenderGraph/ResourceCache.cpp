#include "polypch.h"
#include "ResourceCache.h"

#include "Poly/Core/RenderAPI.h"
#include "Platform/API/Buffer.h"
#include "Platform/API/Texture.h"
#include "Platform/API/TextureView.h"
#include "Platform/API/CommandQueue.h"
#include "Resource.h"

namespace Poly
{
	ResourceCache::ResourceCache(RenderGraphDefaultParams defaultParams)
	{
		m_DefaultParams = defaultParams;
	}

	Ref<ResourceCache> ResourceCache::Create(RenderGraphDefaultParams defaultParams)
	{
		return CreateRef<ResourceCache>(defaultParams);
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

	void ResourceCache::MarkOutput(const std::string& name, IOData iodata)
	{
		auto itr = std::find_if(m_Resources.begin(), m_Resources.end(), [&](const ResourceData& data){ return data.Name == name; });
		if (itr == m_Resources.end()) // Register resource if it hasn't been that already
		{
			RegisterResource(name, 0, iodata);
			m_Resources.back().IsOutput = true;
		}
		else
		{
			itr->IsOutput = true;
		}

	}

	void ResourceCache::SetBackbuffer(Ref<Resource> pResource)
	{
		auto itr = std::find_if(m_Resources.begin(), m_Resources.end(), [&](const ResourceData& data){ return data.IsOutput; });
		if (itr != m_Resources.end())
			itr->pResource = pResource;
	}

	void ResourceCache::AllocateResources()
	{
		for (auto& resourceData : m_Resources)
		{
			// Skip resource marked as output - these are handled separate as they will use externally allocated backbuffer
			if (resourceData.IsOutput)
				continue;

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
				desc.Width			= resourceData.IOInfo.Width == 0 ? m_DefaultParams.TextureWidth : resourceData.IOInfo.Width;
				desc.Height			= resourceData.IOInfo.Height == 0 ? m_DefaultParams.TextureHeight : resourceData.IOInfo.Height;
				desc.Depth			= 1;
				desc.ArrayLayers	= 1;
				desc.MipLevels		= 1; // TODO: Add support for mips
				desc.SampleCount	= 1;
				desc.TextureDim		= ETextureDim::DIM_2D;
				desc.TextureUsage	= bindPoint == FResourceBindPoint::COLOR_ATTACHMENT ? FTextureUsage::COLOR_ATTACHMENT : FTextureUsage::DEPTH_STENCIL_ATTACHMENT;
				desc.Format			= resourceData.IOInfo.Format;
				Ref<Texture> pTexture = RenderAPI::CreateTexture(&desc);

				TextureViewDesc desc2 = {};
				desc2.ArrayLayer		= 0;
				desc2.ArrayLayerCount	= 1;
				desc2.Format			= resourceData.IOInfo.Format;
				desc2.ImageViewFlag		= bindPoint == FResourceBindPoint::COLOR_ATTACHMENT ? FImageViewFlag::COLOR : FImageViewFlag::DEPTH_STENCIL;
				desc2.ImageViewType		= EImageViewType::TYPE_2D;
				desc2.MipLevel			= 0;
				desc2.MipLevelCount		= 1;
				desc2.pTexture			= pTexture.get();
				Ref<TextureView> pTextureView = RenderAPI::CreateTextureView(&desc2);

				resourceData.pResource = Resource::Create(pTexture, pTextureView, resourceData.Name);
				resourceData.pResource->SetSampler(resourceData.IOInfo.pSampler ? resourceData.IOInfo.pSampler : m_DefaultParams.pSampler);
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