#pragma once

#include "RenderGraphTypes.h"

namespace Poly
{
	class Resource;
	class ResourceCache;

	/**
	 * Render Data is created for each execute call of a pass.
	 * The class contains the data that might be needed to execute,
	 * especially the resources with the same name as the reflection.
	 */
	class RenderData
	{
	public:
		RenderData(const std::string& renderPassName, Ref<ResourceCache> pResourceCache, RenderGraphDefaultParams defaultParams);
		~RenderData() = default;

		/**
		 * Get a resource with the same name given in Reflect
		 * @return Resource
		 */
		const Resource* operator[] (const std::string& resourceName) const;

	private:
		Ref<ResourceCache> m_pResourceCache;
		std::string m_RenderPassName = "";
		RenderGraphDefaultParams m_DefaultParams = {};
	};
}