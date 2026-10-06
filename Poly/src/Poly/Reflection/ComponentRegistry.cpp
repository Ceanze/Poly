#include "ComponentRegistry.h"

namespace Poly
{
	bool ComponentRegistry::Add(ComponentDesc desc)
	{
		if (Find(desc.TypeInfo.Name))
		{
			POLY_CORE_WARN("Cannot register component '{}', a component with that name is already registered", desc.TypeInfo.Name);
			return false;
		}

		s_Components.push_back(CreateUnique<ComponentDesc>(std::move(desc)));
		return true;
	}

	const ComponentDesc* ComponentRegistry::Find(std::string_view name)
	{
		for (const Unique<ComponentDesc>& pComponent : s_Components)
		{
			if (pComponent->TypeInfo.Name == name)
				return pComponent.get();
		}

		return nullptr;
	}

	const std::vector<Unique<ComponentDesc>>& ComponentRegistry::GetAll()
	{
		return s_Components;
	}

	std::string ComponentRegistry::GetNames()
	{
		std::string names;
		for (const Unique<ComponentDesc>& pComponent : s_Components)
		{
			if (!names.empty())
				names += ", ";
			names += pComponent->TypeInfo.Name;
		}

		return names;
	}
} // namespace Poly
