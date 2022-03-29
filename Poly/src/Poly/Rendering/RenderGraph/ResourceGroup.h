#pragma once

#include "RenderGraphTypes.h"

namespace Poly
{
	class Resource;

	class ResourceGroup
	{
	public:
		ResourceGroup(const std::string name) : m_GroupName(name) {}
		~ResourceGroup() = default;

		/**
		 * Adds a resource to the group, name is gotten from the resource
		 * @param pResource - resource to be added - must not contain group name
		 */
		void AddResource(Ref<Resource> pResource, bool autoBindDescriptor);

		/**
		 * Adds a resource to the group, resource must be updated
		 * with the render graph program before use
		 * to guarantee an active resource can be used.
		 * If a resource already exists, use AddResource(Ref<Resource>)
		 * @param name - name of resource to be added - must not contain group name
		 */
		void AddResource(const std::string& name, bool autoBindDescriptor);

		/**
		 * Sets the resource of a previously added resource
		 * @param name - name of resource to set - must not contain group name
		 * @param pResource - resource to set
		 */
		void SetResource(const std::string& name, Ref<Resource> pResource);

		/**
		 * Gets a previously added resource.
		 * Nullptr if resource hasn't been added/updated
		 * @return resource
		 */
		Ref<Resource> GetResource(const std::string& name) const;

		/**
		 * Gets a previously added resource.
		 * @return resourceInfo
		 */
		const ResourceInfo& GetResourceInfo(const std::string& name) const;

		/**
		 * Gets all added resources
		 * @return vector of added resources
		 */
		const std::unordered_map<std::string, ResourceInfo>& GetResources() const { return m_Resources; }

		/**
		 * @return name of group
		 */
		const std::string& GetGroupName() const { return m_GroupName; }

	private:
		std::string m_GroupName;
		std::unordered_map<std::string, ResourceInfo> m_Resources;
	};
}