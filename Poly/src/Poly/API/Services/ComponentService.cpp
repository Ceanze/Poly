#include "ComponentService.h"

#include "Poly/API/Commands/ComponentCommands.h"
#include "Poly/API/EngineContext.h"
#include "Poly/Reflection/ComponentRegistry.h"
#include "Poly/Reflection/JsonHelpers.h"

namespace
{
	struct Target
	{
		Poly::Entity               Entity     = Poly::Entity::None();
		const Poly::ComponentDesc* pComponent = nullptr;
	};

	// The entity and one of its components, for the queries. Commands validate on their own
	Poly::Result<Target> FindTarget(const Poly::World& world, Poly::PolyID id, std::string_view component)
	{
		Target target;
		target.Entity = world.FindEntity(id);
		if (!target.Entity.IsValid())
			return Poly::MakeError(Poly::ErrorCode::EntityNotFound, fmt::format("Entity {} does not exist", id));

		target.pComponent = Poly::ComponentRegistry::Find(component);
		if (!target.pComponent)
			return Poly::MakeError(Poly::ErrorCode::ComponentNotFound, fmt::format("Component '{}' is not registered. Available: {}", component, Poly::ComponentRegistry::GetNames()));

		if (!target.pComponent->Has(target.Entity))
			return Poly::MakeError(Poly::ErrorCode::ComponentNotPresent, fmt::format("Entity {} does not have component '{}'", id, component));

		return target;
	}
} // namespace

namespace Poly::API
{
	ComponentService::ComponentService(EngineContext& context)
	    : m_Context(context)
	{}

	Result<void> ComponentService::Add(PolyID id, std::string_view component, std::string_view fieldsJson)
	{
		return m_Context.History().Execute(CreateUnique<AddComponentCommand>(id, component, fieldsJson));
	}

	Result<void> ComponentService::Remove(PolyID id, std::string_view component)
	{
		return m_Context.History().Execute(CreateUnique<RemoveComponentCommand>(id, component));
	}

	Result<void> ComponentService::SetField(PolyID id, std::string_view component, std::string_view field, std::string_view valueJson)
	{
		return m_Context.History().Execute(CreateUnique<SetFieldCommand>(id, component, field, valueJson));
	}

	Result<std::string> ComponentService::Get(PolyID id, std::string_view component) const
	{
		const Result<Target> target = FindTarget(m_Context.GetWorld(), id, component);
		if (!target)
			return std::unexpected(target.error());

		return target->pComponent->ToJson(target->Entity);
	}

	Result<std::string> ComponentService::GetField(PolyID id, std::string_view component, std::string_view field) const
	{
		const Result<std::string> fields = Get(id, component);
		if (!fields)
			return fields;

		return Json::GetMember(*fields, field);
	}

	Result<bool> ComponentService::Inspect(PolyID id, std::string_view component, FieldVisitor& visitor)
	{
		const Result<Target> target = FindTarget(m_Context.GetWorld(), id, component);
		if (!target)
			return std::unexpected(target.error());

		std::vector<FieldEdit> edits;
		target->pComponent->VisitFields(target->Entity, visitor, edits);

		for (const FieldEdit& edit : edits)
		{
			if (Result<void> result = SetField(id, component, edit.Field, edit.Json); !result)
				return std::unexpected(result.error());
		}

		return !edits.empty();
	}

	std::vector<ComponentTypeInfo> ComponentService::GetTypes() const
	{
		std::vector<ComponentTypeInfo> types;
		for (const Unique<ComponentDesc>& pComponent : ComponentRegistry::GetAll())
			types.push_back(pComponent->TypeInfo);

		return types;
	}

	Result<std::vector<std::string>> ComponentService::GetAddable(PolyID id) const
	{
		const Entity entity = m_Context.GetWorld().FindEntity(id);
		if (!entity.IsValid())
			return MakeError(ErrorCode::EntityNotFound, fmt::format("Entity {} does not exist", id));

		std::vector<std::string> addable;
		for (const Unique<ComponentDesc>& pComponent : ComponentRegistry::GetAll())
		{
			if (pComponent->TypeInfo.Addable && !pComponent->Has(entity))
				addable.push_back(pComponent->TypeInfo.Name);
		}

		return addable;
	}
} // namespace Poly::API
