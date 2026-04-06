#include "ResourceCache.h"

#include "Platform/API/Buffer.h"
#include "Platform/API/CommandQueue.h"
#include "Platform/API/Texture.h"
#include "Platform/API/TextureView.h"
#include "Poly/Core/RenderAPI.h"
#include "polypch.h"
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

	void ResourceCache::RegisterExternalResource(const ResID& resID, ResourceInfo resourceInfo)
	{
		const PassResID externalId = resID.GetAsExternal();
		if (m_NameToExternalIndex.contains(externalId))
			m_ExternalResources[m_NameToExternalIndex[externalId]] = resourceInfo;
		else
		{
			m_NameToExternalIndex[externalId] = static_cast<uint32>(m_ExternalResources.size());
			m_ExternalResources.push_back(resourceInfo);
			m_ExternalCanonicalGUIDs.push_back(externalId);
		}
	}

	void ResourceCache::RegisterResource(const PassResID& passResID, uint32 timepoint, PassField passField, const PassResID& aliasID)
	{
		if (m_NameToIndex.contains(passResID))
		{
			POLY_CORE_WARN("Resource {} has already been added, ignoring call", passResID.GetFullName());
			return;
		}

		bool isAlias = aliasID.HasResource();
		if (isAlias && (!m_NameToIndex.contains(aliasID) && !m_NameToExternalIndex.contains(aliasID)))
		{
			POLY_CORE_WARN("Resource {} cannot use alias {}, aliased resource has not been registered", passResID.GetFullName(), aliasID.GetFullName());
			return;
		}

		if (!isAlias) // New resource
		{
			uint32 index             = static_cast<uint32>(m_Resources.size());
			m_NameToIndex[passResID] = index;
			ResourceData data        = {};
			data.Lifetime            = {timepoint, timepoint};
			data.PassResID           = passResID;
			data.PassField           = passField;
			m_Resources.push_back(data);
		}
		else // Aliased resource
		{
			if (m_NameToIndex.contains(aliasID))
			{
				uint32 index             = m_NameToIndex[aliasID];
				m_NameToIndex[passResID] = index;
				m_Resources[index].PassField.Merge(passField);
				CalcLifetime(m_Resources[index].Lifetime, timepoint);
			}
			else
			{
				m_NameToExternalIndex[passResID] = m_NameToExternalIndex[aliasID];
			}
		}
	}

	void ResourceCache::RegisterSyncResource(const PassResID& passResID, const PassResID& aliasID)
	{
		// A sync resource is always an alias
		if (!IsResourceRegistered(aliasID))
		{
			POLY_CORE_WARN("Resource {} cannot use alias {}, alias resource has not been registered", passResID.GetFullName(), aliasID.GetFullName());
			return;
		}

		if (m_NameToIndex.contains(aliasID))
		{
			m_NameToIndex[passResID] = m_NameToIndex[aliasID];
		}
		else
		{
			m_NameToExternalIndex[passResID] = m_NameToExternalIndex[aliasID];
		}
	}

	void ResourceCache::MarkOutput(const PassResID& passResID, PassField passField)
	{
		if (!m_NameToIndex.contains(passResID))
			RegisterResource(passResID, 0, passField);

		ResourceData& resData     = m_Resources.at(m_NameToIndex.at(passResID));
		resData.IsOutput          = true;
		resData.IsBackbufferBound = true;
	}

	void ResourceCache::SetBackbuffer(PolyID windowID, uint32 imageIndex, Ref<Resource> pResource)
	{
		uint32 windowIndex = 0;
		if (auto itr = m_WindowIDtoIndex.find(windowID); itr == m_WindowIDtoIndex.end())
		{
			windowIndex                 = m_Backbuffers.size();
			m_WindowIDtoIndex[windowID] = windowIndex;
		}
		else
			windowIndex = itr->second;

		if (windowIndex >= m_Backbuffers.size())
			m_Backbuffers.resize(windowIndex + 1);

		if (imageIndex >= m_Backbuffers[windowIndex].size())
			m_Backbuffers[windowIndex].resize(imageIndex + 1);

		m_Backbuffers[windowIndex][imageIndex] = std::move(pResource);
	}

	void ResourceCache::SetCurrentBackbufferIndices(PolyID windowID, uint32 imageIndex)
	{
		if (auto itr = m_WindowIDtoIndex.find(windowID); itr == m_WindowIDtoIndex.end())
		{
			m_CurrentWindowIndex        = m_Backbuffers.size();
			m_WindowIDtoIndex[windowID] = m_CurrentWindowIndex;
		}
		else
			m_CurrentWindowIndex = itr->second;

		m_CurrentImageIndex = imageIndex;
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
			if (resourceData.IsOutput)
				continue;

			if (resourceData.IsBackbufferBound)
				AllocateResource(resourceData, width, height);
		}
	}

	bool ResourceCache::HasResource(const PassResID& passResID) const
	{
		if (const auto itr = m_NameToIndex.find(passResID); itr != m_NameToIndex.end())
		{
			const ResourceData& resData = m_Resources[m_NameToIndex.at(passResID)];
			return resData.IsOutput || resData.pResource != nullptr;
		}

		if (const auto itr = m_NameToExternalIndex.find(passResID); itr != m_NameToExternalIndex.end())
		{
			const ResourceInfo& resInfo = m_ExternalResources[m_NameToExternalIndex.at(passResID)];
			return resInfo.pResource != nullptr;
		}

		return false;
	}

	bool ResourceCache::HasResource(const ResID& resID) const
	{
		const PassResID externalId = resID.GetAsExternal();
		if (const auto itr = m_NameToExternalIndex.find(externalId); itr != m_NameToExternalIndex.end())
		{
			const ResourceInfo& resInfo = m_ExternalResources[m_NameToExternalIndex.at(externalId)];
			return resInfo.pResource != nullptr;
		}

		return false;
	}

	bool ResourceCache::IsResourceRegistered(const PassResID& passRessID) const
	{
		return m_NameToIndex.contains(passRessID) || m_NameToExternalIndex.contains(passRessID);
	}

	bool ResourceCache::IsResourceRegistered(const ResID& resID) const
	{
		return m_NameToExternalIndex.contains(resID.GetAsExternal());
	}

	Resource* ResourceCache::GetResource(const PassResID& passResID) const
	{
		if (!HasResource(passResID))
		{
			POLY_CORE_WARN("Resource {} cannot be gotten, it does not exist", passResID.GetFullName());
			return nullptr;
		}

		if (m_NameToIndex.contains(passResID))
		{
			const ResourceData& data = m_Resources[m_NameToIndex.at(passResID)];
			if (data.IsOutput)
				return m_Backbuffers[m_CurrentWindowIndex][m_CurrentImageIndex].get();
			else
				return m_Resources[m_NameToIndex.at(passResID)].pResource.get();
		}
		else
			return m_ExternalResources[m_NameToExternalIndex.at(passResID)].pResource.get();
	}

	Resource* ResourceCache::GetResource(const ResID& resID) const
	{
		if (!HasResource(resID))
		{
			POLY_CORE_WARN("Resource {} cannot be gotten, it does not exist", resID.GetName());
			return nullptr;
		}

		return m_ExternalResources[m_NameToExternalIndex.at(resID.GetAsExternal())].pResource.get();
	}

	PassResID ResourceCache::GetMappedResourceName(const PassResID& resPassID, const PassID& passID)
	{
		// Internal resource
		const auto& fromItr = m_NameToIndex.find(resPassID);
		if (fromItr != m_NameToIndex.end())
		{
			for (const auto& toPair : m_NameToIndex)
			{
				// If the correct node index and the pass name matches then return the mapped resource name
				if (toPair.second == fromItr->second && toPair.first.GetPass() == passID)
					return toPair.first;
			}
		}

		// External resource
		const auto& fromExternalItr = m_NameToExternalIndex.find(resPassID);
		if (fromExternalItr != m_NameToExternalIndex.end())
		{
			for (const auto& toPair : m_NameToExternalIndex)
			{
				// If the correct node index and the pass name matches then return the mapped resource name
				if (toPair.second == fromExternalItr->second && toPair.first.GetPass() == passID)
					return toPair.first;
			}
		}

		return PassResID::Invalid();
	}

	PassResID ResourceCache::GetCanonicalGUID(const PassResID& passResID)
	{
		const auto passResourceItr = m_NameToIndex.find(passResID);
		if (passResourceItr != m_NameToIndex.end())
			return m_Resources[passResourceItr->second].PassResID;

		const auto externalResourceItr = m_NameToExternalIndex.find(passResID);
		if (externalResourceItr != m_NameToExternalIndex.end())
			return m_ExternalCanonicalGUIDs[externalResourceItr->second];

		POLY_CORE_ERROR("Called GetCanonicalGUID with '{}', which is not registered to the cache", passResID.GetFullName());
		return PassResID::Invalid();
	}

	Resource* ResourceCache::UpdateResourceSize(const PassResID& passResID, uint64 size)
	{
		if (!HasResource(passResID))
		{
			POLY_CORE_WARN("Resource {} cannot be updated, it does not exist", passResID.GetFullName());
			return nullptr;
		}

		auto getResourceFunc = [this, &passResID]() -> Ref<Resource>& {
			if (m_NameToIndex.contains(passResID))
				return m_Resources[m_NameToIndex[passResID]].pResource;
			else
				return m_ExternalResources[m_NameToExternalIndex[passResID]].pResource;
		};

		Ref<Resource>& pRes = getResourceFunc();

		if (!pRes)
			return nullptr;

		if (pRes->IsBuffer())
		{
			Buffer* pBuffer = pRes->GetAsBuffer();

			auto desc       = pBuffer->GetDesc();
			desc.Size       = size;
			auto pNewBuffer = RenderAPI::CreateBuffer(&desc);

			Ref<Resource> pNewResource = Resource::Create(pNewBuffer, passResID.GetResource().GetName());

			pRes.swap(pNewResource);
			pNewResource.reset();

			return pRes.get();
		}

		if (pRes->IsTexture())
		{
			POLY_CORE_ERROR("TODO: Add texture resizing support");
		}

		POLY_CORE_ERROR("Resource {} was neither a Buffer nor a Texture, cannot update size", passResID.GetFullName());
		return nullptr;
	}

	void ResourceCache::AddBindpoint(const PassResID& passResID, FResourceBindPoint additionalBindpoint)
	{
		if (!m_NameToIndex.contains(passResID))
			return;

		uint32    index = m_NameToIndex.at(passResID);
		PassField extra;
		extra.BindPoint(additionalBindpoint);
		m_Resources[index].PassField.Merge(extra);
	}

	void ResourceCache::Reset()
	{
		m_NameToIndex.clear();
		m_NameToExternalIndex.clear();

		// Wait for GPU since graphic objects might be deleted
		RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Wait();
		m_Resources.clear();
		m_ExternalResources.clear();
		m_ExternalCanonicalGUIDs.clear();
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
			BufferDesc desc  = {};
			desc.BufferUsage = bindPoint == FResourceBindPoint::STORAGE ? FBufferUsage::STORAGE_BUFFER : FBufferUsage::UNIFORM_BUFFER;
			desc.MemUsage    = EMemoryUsage::GPU_ONLY; // TODO: Check if staging buffers should/can be created here
			desc.Size        = resourceData.PassField.GetSize();

			resourceData.pResource = Resource::Create(RenderAPI::CreateBuffer(&desc), resourceData.PassResID.GetResource().GetName());
		}
		else if (BitsSet(bindPoint, FResourceBindPoint::COLOR_ATTACHMENT) || BitsSet(bindPoint, FResourceBindPoint::DEPTH_STENCIL) || BitsSet(bindPoint, FResourceBindPoint::SAMPLER) || BitsSet(bindPoint, FResourceBindPoint::SHADER_READ))
		{
			uint32 width  = resourceData.PassField.GetWidth() == 0 ? m_DefaultParams.TextureWidth : resourceData.PassField.GetWidth();
			uint32 height = resourceData.PassField.GetHeight() == 0 ? m_DefaultParams.TextureHeight : resourceData.PassField.GetHeight();

			if (BitsSet(bindPoint, FResourceBindPoint::DEPTH_STENCIL))
				resourceData.IsBackbufferBound = true;

			if (resourceData.IsBackbufferBound)
			{
				width  = backbufferWidth;
				height = backbufferHeight;
			}

			TextureDesc desc      = {};
			desc.DebugName        = resourceData.PassResID.GetResource().GetName();
			desc.Width            = width;
			desc.Height           = height;
			desc.Depth            = resourceData.PassField.GetDepth() == 0 ? m_DefaultParams.TextureDepth : resourceData.PassField.GetDepth();
			desc.ArrayLayers      = 1;
			desc.MipLevels        = 1; // TODO: Add support for mips
			desc.SampleCount      = 1;
			desc.TextureDim       = ETextureDim::DIM_2D;
			desc.TextureUsage     = ConvertResourceBindPointToTextureUsage(bindPoint);
			desc.Format           = resourceData.PassField.GetFormat() != EFormat::UNDEFINED ? resourceData.PassField.GetFormat() : m_DefaultParams.Format;
			Ref<Texture> pTexture = RenderAPI::CreateTexture(&desc);

			TextureViewDesc desc2         = {};
			desc2.DebugName               = resourceData.PassResID.GetResource().GetName();
			desc2.ArrayLayer              = 0;
			desc2.ArrayLayerCount         = 1;
			desc2.Format                  = resourceData.PassField.GetFormat() != EFormat::UNDEFINED ? resourceData.PassField.GetFormat() : m_DefaultParams.Format;
			desc2.ImageViewFlag           = BitsSet(bindPoint, FResourceBindPoint::DEPTH_STENCIL) ? FImageViewFlag::DEPTH_STENCIL : FImageViewFlag::COLOR;
			desc2.ImageViewType           = EImageViewType::TYPE_2D;
			desc2.MipLevel                = 0;
			desc2.MipLevelCount           = 1;
			desc2.pTexture                = pTexture.get();
			Ref<TextureView> pTextureView = RenderAPI::CreateTextureView(&desc2);

			resourceData.pResource = Resource::Create(pTexture, pTextureView, resourceData.PassResID.GetResource().GetName());
			resourceData.pResource->SetSampler(resourceData.PassField.GetSampler() ? resourceData.PassField.GetSampler() : m_DefaultParams.pSampler);
		}
	}
} // namespace Poly