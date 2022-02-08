#pragma once

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
		 * @param pResource - resource to be added
		 */
		void AddResource(Ref<Resource> pResource);

		/**
		 * Adds a resource to the group, resource must be updated
		 * with the render graph program before use
		 * to guarantee an active resource can be used.
		 * If a resource already exists, use AddResource(Ref<Resource>)
		 * @param name - name of resource to be added
		 */
		void AddResource(const std::string& name);

		/**
		 * Gets a previously added resource.
		 * Nullptr if resource hasn't been added/updated
		 * @return resource
		 */
		Ref<Resource> GetResource(const std::string& name) const;

		/**
		 * Gets all added resources
		 * @return vector of added resources
		 */
		const std::unordered_map<std::string, Ref<Resource>>& GetResources() const { return m_Resources; }

		/**
		 * @return name of group
		 */
		const std::string& GetGroupName() const { return m_GroupName; }

	private:
		std::string m_GroupName;
		std::unordered_map<std::string, Ref<Resource>> m_Resources;
	};
}