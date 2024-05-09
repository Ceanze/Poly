#pragma once

#include "ResourceGUID.h"
#include "PassReflection.h"
#include "RenderGraphTypes.h"

namespace Poly
{
	class Resource;

	class ResourceCache
	{
	private:
		struct ResourceData
		{
			std::pair<uint32, uint32>	Lifetime	= {0, 0};
			Ref<Resource>				pResource	= nullptr;
			std::string					Name		= "";
			IOData						IOInfo		= {};
			bool						IsOutput	= false;
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
		void RegisterExternalResource(const std::string& name, ResourceInfo resourceInfo);

		/**
		 * Registers a resource to be created
		 * @param name - name of resource following renderPass.resourceName format
		 * @param timepoint - index of render pass (from execution order)
		 * @param iodata - IOData for the resource
		 * @param alias - [optional] informs that the resource is going by another name added previously
		 */
		void RegisterResource(const std::string& name, uint32 timepoint, IOData iodata, const std::string& alias = "");

		void RegisterSyncResource(const std::string& name, const std::string& alias);

		/**
		 * Mark a resource as being the output of the graph - this will
		 * make it and all aliases to it an alias to the backbuffer for each frame
		 * @param name - name of resource following renderPass.resourceName format
		 * @param iodata - IOData for that resource
		 */
		void MarkOutput(const std::string& name, IOData iodata);

		/**
		 * Sets the current backbuffer resource for this frame to be used
		 * @param pResource - current backbuffer
		 */
		void SetBackbuffer(Ref<Resource> pResource);

		/**
		 * Allocated the previously registered resoruces
		 */
		void AllocateResources();

		/**
		 * Get a resource
		 * @param name - resource name
		 * 
		 * @return pointer to resource
		 */
		Resource* GetResource(const std::string& name);

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
		Resource* UpdateResourceSize(const std::string& name, uint64 size);

		/**
		 * Resets the cache, losing ownership of resource and clears vectors
		 */
		void Reset();

	private:
		void CalcLifetime(std::pair<uint32, uint32>& lifetime, uint32 newTimepoint);

		RenderGraphDefaultParams m_DefaultParams;

		std::unordered_map<std::string, uint32> m_NameToIndex;
		std::vector<ResourceData> m_Resources;
		std::unordered_map<std::string, uint32> m_NameToExternalIndex;
		std::vector<ResourceInfo> m_ExternalResources;
	};
}