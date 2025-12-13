#pragma once

#include <unordered_map>

#include "ResourceGUID.h"
#include "RenderGraphTypes.h"
#include "Poly/Rendering/RenderGraph/Reflection/PassField.h"

namespace Poly
{
	class Resource;

	class ResourceCache
	{
	private:
		struct ResourceData
		{
			std::pair<uint32, uint32>	Lifetime			= {0, 0};
			Ref<Resource>				pResource			= nullptr;
			ResourceGUID				ResourceGUID		= ResourceGUID::Invalid();
			PassField					PassField			= {};
			bool						IsOutput			= false;
			bool						IsBackbufferBound	= false;
		};

	public:
		ResourceCache(RenderGraphDefaultParams defaultParams);
		~ResourceCache() = default;

		static Ref<ResourceCache> Create(RenderGraphDefaultParams defaultParams);

		/**
		 * Register an external resource which is not handled by the resource cache
		 * External resources are expected to be alive during the render graphs lifespan
		 * Additional calls to this function with same name will update the resource pointer
		 * @param name - name of resource following $.resourceName format
		 * @param resourceInfo - resource info (pResource & autobind)
		 */
		void RegisterExternalResource(const ResourceGUID& resourceGUID, ResourceInfo resourceInfo);

		/**
		 * Registers a resource to be created
		 * @param name - name of resource following renderPass.resourceName format
		 * @param timepoint - index of render pass (from execution order)
		 * @param iodata - IOData for the resource
		 * @param alias - [optional] informs that the resource is going by another name added previously
		 */
		void RegisterResource(const ResourceGUID& resourceGUID, uint32 timepoint, PassField iodata, const ResourceGUID& aliasGUID = ResourceGUID::Invalid());

		void RegisterSyncResource(const ResourceGUID& resourceGUID, const ResourceGUID& aliasGUID);

		/**
		 * Mark a resource as being the output of the graph - this will
		 * make it and all aliases to it an alias to the backbuffer for each frame
		 * @param name - name of resource following renderPass.resourceName format
		 * @param iodata - IOData for that resource
		 */
		void MarkOutput(const ResourceGUID& resourceGUID, PassField iodata);

		/**
		 * Sets the backbuffer resource for the specified window/imageIndex pair.
		 * NOTE: Needs to be re-set each time a the backbuffer becomes stale (e.g. when resizing the window)
		 * @param windowID - the window to set the backbuffer for
		 * @param imageIndex - the image index for the specified window to set the backbuffer for
		 * @param pResource - current backbuffer
		 */
		void SetBackbuffer(PolyID windowID, uint32 imageIndex, Ref<Resource> pResource);

		/**
		* Sets the current back buffer indices
		* @param windowId - the current window ID
		* @param imageIndex - the current image index for the specified window
		*/
		void SetCurrentBackbufferIndices(PolyID windowID, uint32 imageIndex);

		/**
		 * Allocated the previously registered resoruces
		 */
		void AllocateResources();

		/**
		* Reallocate the previously registed backbuffer bound resources, needed when a resize has happened
		*/
		void ReallocateBackbufferBoundResources(uint32 width, uint32 height);

		/**
		 * Get a resource's existance
		 * @param resourceGUID - resource name
		 *
		 * @return true if resource exist, false otherwise
		 */
		bool HasResource(const ResourceGUID& resourceGUID) const;

		/**
		* Checks if a resource is registered (but not necessarily created)
		* @param resourceGUID - resource name
		* 
		* @return true if resource is registered, false otherwise
		*/
		bool IsResourceRegistered(const ResourceGUID& resourceGUID) const;

		/**
		 * Get a resource
		 * @param resourceGUID - resource guid
		 * 
		 * @return pointer to resource
		 */
		Resource* GetResource(const ResourceGUID& resourceGUID);

		/**
		* Gets the mapped resource name, i.e. resource "pass.resource" retruns the mapped name for "passName.resource"
		* @param resource - resource to map from, following the "passName.resource" syntax
		* @param passName - the pass to map to, following "passName" syntax, i.e. no resource suffix
		* 
		* @return "resource" name that belongs to the pass
		*/
		ResourceGUID GetMappedResourceName(const ResourceGUID& resourceGUID, const std::string& passName);

		/**
		* Update a resource size
		* WARNING: Old data will be deleted when size is changed. 
		*/
		Resource* UpdateResourceSize(const ResourceGUID& resourceGUID, uint64 size);

		/**
		 * Resets the cache, losing ownership of resource and clears vectors
		 */
		void Reset();

	private:
		void CalcLifetime(std::pair<uint32, uint32>& lifetime, uint32 newTimepoint);
		void AllocateResource(ResourceData& resourceData, uint32 backbufferWidth, uint32 backbufferHeight);

		RenderGraphDefaultParams m_DefaultParams;

		std::unordered_map<ResourceGUID, uint32, ResourceGUIDHasher> m_NameToIndex;
		std::vector<ResourceData> m_Resources;
		std::unordered_map<ResourceGUID, uint32, ResourceGUIDHasher> m_NameToExternalIndex;
		std::vector<ResourceInfo> m_ExternalResources;
		std::vector<std::vector<Ref<Resource>>> m_Backbuffers;
		std::unordered_map<PolyID, uint32> m_WindowIDtoIndex;
		uint32 m_CurrentWindowIndex = 0;
		uint32 m_CurrentImageIndex = 0;
	};
}