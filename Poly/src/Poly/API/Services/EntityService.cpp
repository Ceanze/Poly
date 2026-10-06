#include "EntityService.h"

#include "Poly/API/Commands/ComponentCommands.h"
#include "Poly/API/Commands/EntityCommands.h"
#include "Poly/API/EngineContext.h"
#include "Poly/Reflection/ComponentRegistry.h"
#include "Poly/Reflection/JsonHelpers.h"
#include "Poly/Scene/Components/NameComponent.h"

namespace Poly::API
{
	EntityService::EntityService(EngineContext& context)
	    : m_Context(context)
	{}

	Result<PolyID> EntityService::Create(PolyID parent, std::string_view name, uint32 siblingIndex)
	{
		Unique<CreateEntityCommand> pCommand = CreateUnique<CreateEntityCommand>(parent, name, siblingIndex);

		// The history owns the command once it has been executed
		CreateEntityCommand* pCreated = pCommand.get();
		if (Result<void> result = m_Context.History().Execute(std::move(pCommand)); !result)
			return std::unexpected(result.error());

		return pCreated->GetID();
	}

	Result<void> EntityService::Destroy(PolyID id)
	{
		return m_Context.History().Execute(CreateUnique<DestroyEntityCommand>(id));
	}

	Result<void> EntityService::SetParent(PolyID id, PolyID parent, uint32 siblingIndex)
	{
		return m_Context.History().Execute(CreateUnique<SetParentCommand>(id, parent, siblingIndex));
	}

	Result<void> EntityService::Rename(PolyID id, std::string_view name)
	{
		const Entity entity = m_Context.GetWorld().FindEntity(id);
		if (!entity.IsValid())
			return MakeError(ErrorCode::EntityNotFound, fmt::format("Cannot rename entity, {} does not exist", id));

		if (entity.HasComponent<NameComponent>())
			return m_Context.History().Execute(CreateUnique<SetFieldCommand>(id, "Name", "Name", Json::Quote(name)));

		return m_Context.History().Execute(CreateUnique<AddComponentCommand>(id, "Name", Json::MakeObject("Name", Json::Quote(name))));
	}

	Result<EntityInfo> EntityService::Get(PolyID id) const
	{
		Entity entity = m_Context.GetWorld().FindEntity(id);
		if (!entity.IsValid())
			return MakeError(ErrorCode::EntityNotFound, fmt::format("Entity {} does not exist", id));

		EntityInfo info;
		info.ID = id;

		if (Entity parent = entity.GetParent(); parent.IsValid())
			info.Parent = parent.GetPolyID();

		if (entity.HasComponent<NameComponent>())
			info.Name = entity.GetComponent<NameComponent>().Name;

		for (Entity child : entity.GetChildren())
			info.Children.push_back(child.GetPolyID());

		for (const Unique<ComponentDesc>& pComponent : ComponentRegistry::GetAll())
		{
			if (pComponent->Has(entity))
				info.Components.push_back(pComponent->TypeInfo.Name);
		}

		return info;
	}

	bool EntityService::Exists(PolyID id) const
	{
		return m_Context.GetWorld().FindEntity(id).IsValid();
	}

	std::vector<PolyID> EntityService::GetRoots() const
	{
		std::vector<PolyID> roots;
		for (auto [entity, id, hierarchy] : m_Context.GetWorld().View<IDComponent, HierarchyComponent>().each())
		{
			if (hierarchy.Parent == entt::null)
				roots.push_back(id.ID);
		}

		// Views iterate newest first, creation order is what is expected of a list of entities
		std::reverse(roots.begin(), roots.end());
		return roots;
	}
} // namespace Poly::API
