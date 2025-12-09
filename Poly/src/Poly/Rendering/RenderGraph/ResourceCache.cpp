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

	void ResourceCache::RegisterExternalResource(const ResourceGUID& resourceGUID, ResourceInfo resourceInfo)
	{
		if (m_NameToExternalIndex.contains(resourceGUID))
			m_ExternalResources[m_NameToExternalIndex[resourceGUID]] = resourceInfo;
		else
		{
			m_NameToExternalIndex[resourceGUID] = static_cast<uint32>(m_ExternalResources.size());
			m_ExternalResources.push_back(resourceInfo);
		}
	}

	void ResourceCache::RegisterResource(const ResourceGUID& resourceGUID, uint32 timepoint, PassField passField, const ResourceGUID& aliasGUID)
	{
		if (m_NameToIndex.contains(resourceGUID))
		{
			POLY_CORE_WARN("Resource {} has already been added, ignoring call", resourceGUID.GetFullName());
			return;
		}

		bool isAlias = aliasGUID.HasResource();
		if (isAlias && (!m_NameToIndex.contains(aliasGUID) && !m_NameToExternalIndex.contains(aliasGUID)))
		{
			POLY_CORE_WARN("Resource {} cannot use alias {}, alias resource has not been registered", resourceGUID.GetFullName(), aliasGUID.GetFullName());
			return;
		}

		if (!isAlias) // New resource
		{
			uint32 index = static_cast<uint32>(m_Resources.size());
			m_NameToIndex[resourceGUID] = index;
			ResourceData data = {};
			data.Lifetime		= {timepoint, timepoint};
			data.ResourceGUID	= resourceGUID;
			data.PassField		= passField;
			m_Resources.push_back(data);
		}
		else // Aliased resource
		{
			if (m_NameToIndex.contains(aliasGUID))
			{
				uint32 index = m_NameToIndex[aliasGUID];
				m_NameToIndex[resourceGUID] = index;
				m_Resources[index].PassField.Merge(passField);
				CalcLifetime(m_Resources[index].Lifetime, timepoint);
			}
			else
			{
				m_NameToExternalIndex[resourceGUID] = m_NameToExternalIndex[aliasGUID];
			}
		}
	}

	void ResourceCache::RegisterSyncResource(const ResourceGUID& resourceGUID, const ResourceGUID& aliasGUID)
	{
		// A sync resource is always an alias
		if (!IsResourceRegistered(aliasGUID))
		{
			POLY_CORE_WARN("Resource {} cannot use alias {}, alias resource has not been registered", resourceGUID.GetFullName(), aliasGUID.GetFullName());
			return;
		}

		if (m_NameToIndex.contains(aliasGUID))
		{
			m_NameToIndex[resourceGUID] = m_NameToIndex[aliasGUID];
		}
		else
		{
			m_NameToExternalIndex[resourceGUID] = m_NameToExternalIndex[aliasGUID];
		}
	}

	void ResourceCache::MarkOutput(const ResourceGUID& resourceGUID, PassField passField)
	{
		if (!m_NameToIndex.contains(resourceGUID))
			RegisterResource(resourceGUID, 0, passField);

		ResourceData& resData = m_Resources.at(m_NameToIndex.at(resourceGUID));
		resData.IsOutput = true;
		resData.IsBackbufferBound = true;
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
			AllocateResource(resourceData, m_DefaultParams.TextureWidth, m_DefaultParams.TextureHeight);
		}
	}

	void ResourceCache::ReallocateBackbufferBoundResources(uint32 width, uint32 height)
	{
		for (auto& resourceData : m_Resources)
		{
			if (resourceData.IsBackbufferBound)
				AllocateResource(resourceData, width, height);
		}
	}

	bool ResourceCache::HasResource(const ResourceGUID& resourceGUID) const
	{
		bool exist = false;
		exist |= m_NameToIndex.contains(resourceGUID) && m_Resources[m_NameToIndex.at(resourceGUID)].pResource;
		exist |= m_NameToExternalIndex.contains(resourceGUID) && m_ExternalResources[m_NameToExternalIndex.at(resourceGUID)].pResource;
		return exist;
	}

	bool ResourceCache::IsResourceRegistered(const ResourceGUID& resourceGUID) const
	{
		return m_NameToIndex.contains(resourceGUID) || m_NameToExternalIndex.contains(resourceGUID);
	}

	Resource* ResourceCache::GetResource(const ResourceGUID& resourceGUID)
	{
		if (!HasResource(resourceGUID))
		{
			POLY_CORE_WARN("Resource {} cannot be gotten, it does not exist", resourceGUID.GetFullName());
			return nullptr;
		}

		if (m_NameToIndex.contains(resourceGUID))
			return m_Resources[m_NameToIndex[resourceGUID]].pResource.get();
		else
			return m_ExternalResources[m_NameToExternalIndex[resourceGUID]].pResource.get();
	}

	ResourceGUID ResourceCache::GetMappedResourceName(const ResourceGUID& resourceGUID, const std::string& passName)
	{
		// Find resource index
		const std::string fullResourceName = resourceGUID.GetFullName();
		auto* map = &m_NameToIndex;
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

	Resource* ResourceCache::UpdateResourceSize(const ResourceGUID& resourceGUID, uint64 size)
	{
		if (!HasResource(resourceGUID))
		{
			POLY_CORE_WARN("Resource {} cannot be updated, it does not exist", resourceGUID.GetFullName());
			return nullptr;
		}

		auto getResourceFunc = [this, &resourceGUID]() -> Ref<Resource>&
			{
				if (m_NameToIndex.contains(resourceGUID))
					return m_Resources[m_NameToIndex[resourceGUID]].pResource;
				else
					return m_ExternalResources[m_NameToExternalIndex[resourceGUID]].pResource;
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

			Ref<Resource> pNewResource = Resource::Create(pNewBuffer, resourceGUID.GetResourceName());

			pRes.swap(pNewResource);
			pNewResource.reset();

			return pRes.get();
		}

		if (pRes->IsTexture())
		{
			POLY_CORE_ERROR("TODO: Add texture resizing support");
		}

		POLY_CORE_ERROR("Resource {} was neither a Buffer nor a Texture, cannot update size", resourceGUID.GetFullName());
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

	void ResourceCache::AllocateResource(ResourceData& resourceData, uint32 backbufferWidth, uint32 backbufferHeight)
	{
		// Skip resource marked as output - these are handled separate as they will use externally allocated backbuffer
		if (resourceData.IsOutput)
			return;

		FResourceBindPoint bindPoint = resourceData.PassField.GetBindPoint();
		if (BitsSet(bindPoint, FResourceBindPoint::INTERNAL_USE))
			return;

		if (bindPoint == FResourceBindPoint::STORAGE || bindPoint == FResourceBindPoint::UNIFORM)
		{
			BufferDesc desc = {};
			desc.BufferUsage = bindPoint == FResourceBindPoint::STORAGE ? FBufferUsage::STORAGE_BUFFER : FBufferUsage::UNIFORM_BUFFER;
			desc.MemUsage = EMemoryUsage::GPU_ONLY; // TODO: Check if staging buffers should/can be created here
			desc.Size = resourceData.PassField.GetSize();

			resourceData.pResource = Resource::Create(RenderAPI::CreateBuffer(&desc), resourceData.ResourceGUID.GetResourceName());
		}
		else if (BitsSet(bindPoint, FResourceBindPoint::COLOR_ATTACHMENT) || BitsSet(bindPoint, FResourceBindPoint::DEPTH_STENCIL)
			|| BitsSet(bindPoint, FResourceBindPoint::SAMPLER) || BitsSet(bindPoint, FResourceBindPoint::SHADER_READ))
		{
			uint32 width = resourceData.PassField.GetWidth() == 0 ? m_DefaultParams.TextureWidth : resourceData.PassField.GetWidth();
			uint32 height = resourceData.PassField.GetHeight() == 0 ? m_DefaultParams.TextureHeight : resourceData.PassField.GetHeight();

			if (BitsSet(bindPoint, FResourceBindPoint::DEPTH_STENCIL))
				resourceData.IsBackbufferBound = true;

			if (resourceData.IsBackbufferBound)
			{
				width = backbufferWidth;
				height = backbufferHeight;
			}

			TextureDesc desc = {};
			desc.Width = width;
			desc.Height = height;
			desc.Depth = resourceData.PassField.GetDepth() == 0 ? m_DefaultParams.TextureDepth : resourceData.PassField.GetDepth();
			desc.ArrayLayers = 1;
			desc.MipLevels = 1; // TODO: Add support for mips
			desc.SampleCount = 1;
			desc.TextureDim = ETextureDim::DIM_2D;
			desc.TextureUsage = ConvertResourceBindPointToTextureUsage(bindPoint);
			desc.Format = resourceData.PassField.GetFormat() != EFormat::UNDEFINED ? resourceData.PassField.GetFormat() : m_DefaultParams.Format;
			Ref<Texture> pTexture = RenderAPI::CreateTexture(&desc);

			TextureViewDesc desc2 = {};
			desc2.ArrayLayer = 0;
			desc2.ArrayLayerCount = 1;
			desc2.Format = resourceData.PassField.GetFormat() != EFormat::UNDEFINED ? resourceData.PassField.GetFormat() : m_DefaultParams.Format;
			desc2.ImageViewFlag = bindPoint == FResourceBindPoint::DEPTH_STENCIL ? FImageViewFlag::DEPTH_STENCIL : FImageViewFlag::COLOR;
			desc2.ImageViewType = EImageViewType::TYPE_2D;
			desc2.MipLevel = 0;
			desc2.MipLevelCount = 1;
			desc2.pTexture = pTexture.get();
			Ref<TextureView> pTextureView = RenderAPI::CreateTextureView(&desc2);

			resourceData.pResource = Resource::Create(pTexture, pTextureView, resourceData.ResourceGUID.GetResourceName());
			resourceData.pResource->SetSampler(resourceData.PassField.GetSampler() ? resourceData.PassField.GetSampler() : m_DefaultParams.pSampler);
		}
	}
}