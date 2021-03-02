#pragma once

#include "RenderPassReflection.h"

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
		};

	public:
		ResourceCache() = default;
		~ResourceCache() = default;

		static Ref<ResourceCache> Create();

		/**
		 * Registers a resource to be created
		 * @param name - name of resource following renderPass.resourceName format
		 * @param timepoint - index of render pass (from execution order)
		 * @param iodata - IOData for the resource
		 * @param alias - [optional] informs that the resource is going by another name added previously
		 */
		void RegisterResource(const std::string& name, uint32 timepoint, IOData iodata, const std::string& alias = "");

		/**
		 * Allocated the previously registered resoruces
		 */
		void AllocateResources();

		/**
		 * Get a resource
		 * @param name - resource name
		 */
		Ref<Resource> GetResource(const std::string& name);

	private:
		void CalcLifetime(std::pair<uint32, uint32>& lifetime, uint32 newTimepoint);

		std::unordered_map<std::string, uint32> m_NameToIndex;
		std::vector<ResourceData> m_Resources;
	};
}