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

	void ResourceCache::RegisterExternalResource(const std::string& name, ResourceInfo resourceInfo)
	{
		if (m_NameToExternalIndex.contains(name))
			m_ExternalResources[m_NameToExternalIndex[name]] = resourceInfo;
		else
		{
			m_NameToExternalIndex[name] = m_ExternalResources.size();
			m_ExternalResources.push_back(resourceInfo);
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
				m_Resources[index].IOInfo.Merge(iodata);
				CalcLifetime(m_Resources[index].Lifetime, timepoint);
			}
			else
			{
				m_NameToExternalIndex[name] = m_NameToExternalIndex[alias];
			}
		}
	}

	void ResourceCache::RegisterSyncResource(const std::string& name, const std::string& alias)
	{
		// A sync resource is always an alias
		if (!m_NameToIndex.contains(alias) && !m_NameToExternalIndex.contains(alias))
		{
			POLY_CORE_WARN("Resource {} cannot use alias {}, alias resource has not been registered", name, alias);
			return;
		}

		if (m_NameToIndex.contains(alias))
		{
			m_NameToIndex[name] = m_NameToIndex[alias];
		}
		else
		{
			m_NameToExternalIndex[name] = m_NameToExternalIndex[alias];
		}
	}

	void ResourceCache::MarkOutput(const std::string& name, IOData iodata)
	{
		// auto itr = std::find_if(m_Resources.begin(), m_Resources.end(), [&](const ResourceData& data){ return data.Name == name; });
		if (!m_NameToIndex.contains(name)) // Register resource if it hasn't been that already
		{
			RegisterResource(name, 0, iodata);
			m_Resources.back().IsOutput = true;
		}
		else
		{
			m_Resources[m_NameToIndex[name]].IsOutput = true;
			// itr->IsOutput = true;
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
			if (BitsSet(bindPoint, FResourceBindPoint::INTERNAL_USE))
				continue;

			if (bindPoint == FResourceBindPoint::STORAGE || bindPoint == FResourceBindPoint::UNIFORM)
			{
				BufferDesc desc = {};
				desc.BufferUsage	= bindPoint == FResourceBindPoint::STORAGE ? FBufferUsage::STORAGE_BUFFER : FBufferUsage::UNIFORM_BUFFER;
				desc.MemUsage		= EMemoryUsage::GPU_ONLY; // TODO: Check if staging buffers should/can be created here
				desc.Size			= resourceData.IOInfo.Size;

				resourceData.pResource = Resource::Create(RenderAPI::CreateBuffer(&desc), resourceData.Name);
			}
			else if (BitsSet(bindPoint, FResourceBindPoint::COLOR_ATTACHMENT) || BitsSet(bindPoint, FResourceBindPoint::DEPTH_STENCIL)
					|| BitsSet(bindPoint, FResourceBindPoint::SAMPLER) || BitsSet(bindPoint, FResourceBindPoint::SHADER_READ))
			{
				TextureDesc desc = {};
				desc.Width			= resourceData.IOInfo.Width == 0 ? m_DefaultParams.TextureWidth : resourceData.IOInfo.Width;
				desc.Height			= resourceData.IOInfo.Height == 0 ? m_DefaultParams.TextureHeight : resourceData.IOInfo.Height;
				desc.Depth			= 1;
				desc.ArrayLayers	= 1;
				desc.MipLevels		= 1; // TODO: Add support for mips
				desc.SampleCount	= 1;
				desc.TextureDim		= ETextureDim::DIM_2D;
				desc.TextureUsage	= ConvertResourceBindPointToTextureUsage(bindPoint);
				desc.Format			= resourceData.IOInfo.Format;
				Ref<Texture> pTexture = RenderAPI::CreateTexture(&desc);

				TextureViewDesc desc2 = {};
				desc2.ArrayLayer		= 0;
				desc2.ArrayLayerCount	= 1;
				desc2.Format			= resourceData.IOInfo.Format;
				desc2.ImageViewFlag		= bindPoint == FResourceBindPoint::DEPTH_STENCIL ? FImageViewFlag::DEPTH_STENCIL : FImageViewFlag::COLOR;
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

	Resource* ResourceCache::GetResource(const std::string& name)
	{
		if (!m_NameToIndex.contains(name) && !m_NameToExternalIndex.contains(name))
		{
			POLY_CORE_WARN("Resource {} cannot be gotten, it does not exist", name);
			return nullptr;
		}

		if (m_NameToIndex.contains(name))
			return m_Resources[m_NameToIndex[name]].pResource.get();
		else
			return m_ExternalResources[m_NameToExternalIndex[name]].pResource.get();
	}

	ResourceGUID ResourceCache::GetMappedResourceName(const ResourceGUID& resourceGUID, const std::string& passName)
	{
		// Find resource index
		const std::string fullResourceName = resourceGUID.GetFullName();
		std::unordered_map<std::string, uint32>* map = &m_NameToIndex;
		if (!map->contains(fullResourceName))
		{
			map = &m_NameToExternalIndex;
			if (!map->contains(fullResourceName))
				return ResourceGUID::Invalid();
		}

		const uint32 index = map->at(fullResourceName);

		// Find if passName uses resource
		auto resNameItr = std::find_if(map->begin(), map->end(), [&index, &passName](const auto& pair) {
			auto resGUID = ResourceGUID(pair.first);
			if (pair.second == index && resGUID.GetPassName() == passName)
				return true;
			else
				return false;
			});

		if (resNameItr != map->end())
			return ResourceGUID(resNameItr->first);
		else
			return ResourceGUID::Invalid();
	}

	Resource* ResourceCache::UpdateResourceSize(const std::string& name, uint64 size)
	{
		if (!m_NameToIndex.contains(name) && !m_NameToExternalIndex.contains(name))
		{
			POLY_CORE_WARN("Resource {} cannot be updated, it does not exist", name);
			return nullptr;
		}

		auto getResourceFunc = [this, &name]() -> Ref<Resource>&
			{
				if (m_NameToIndex.contains(name))
					return m_Resources[m_NameToIndex[name]].pResource;
				else
					return m_ExternalResources[m_NameToExternalIndex[name]].pResource;
			};

		Ref<Resource>& pRes = getResourceFunc();

		if (!pRes)
			return nullptr;

		if (pRes->IsBuffer())
		{
			Buffer* pBuffer = pRes->GetAsBuffer();

			auto desc = pBuffer->GetDesc();
			desc.Size = size;
			auto pNewBuffer = RenderAPI::CreateBuffer(&desc);

			Ref<Resource> pNewResource = Resource::Create(pNewBuffer, name);

			pRes.swap(pNewResource);
			pNewResource.reset();

			return pRes.get();
		}

		if (pRes->IsTexture())
		{
			POLY_CORE_ERROR("TODO: Add texture resizing support");
		}

		POLY_CORE_ERROR("Resource {} was neither a Buffer nor a Texture, cannot update size", name);
		return nullptr;
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