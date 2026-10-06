#include "ComponentCommands.h"

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

	Poly::Result<Target> FindTarget(Poly::API::EngineContext& context, Poly::PolyID id, std::string_view component)
	{
		Target target;
		target.Entity = context.GetWorld().FindEntity(id);
		if (!target.Entity.IsValid())
			return Poly::MakeError(Poly::ErrorCode::EntityNotFound, fmt::format("Entity {} does not exist", id));

		target.pComponent = Poly::ComponentRegistry::Find(component);
		if (!target.pComponent)
			return Poly::MakeError(Poly::ErrorCode::ComponentNotFound, fmt::format("Component '{}' is not registered. Available: {}", component, Poly::ComponentRegistry::GetNames()));

		return target;
	}
} // namespace

namespace Poly::API
{
	AddComponentCommand::AddComponentCommand(PolyID id, std::string_view component, std::string_view fieldsJson)
	    : m_ID(id)
	    , m_Component(component)
	    , m_FieldsJson(fieldsJson)
	{}

	Result<void> AddComponentCommand::Execute(EngineContext& context)
	{
		const Result<Target> target = FindTarget(context, m_ID, m_Component);
		if (!target)
			return std::unexpected(target.error());

		if (!target->pComponent->TypeInfo.Addable)
			return MakeError(ErrorCode::ComponentNotAddable, fmt::format("Component '{}' cannot be added to entities", m_Component));

		if (target->pComponent->Has(target->Entity))
			return MakeError(ErrorCode::ComponentAlreadyPresent, fmt::format("Entity {} already has component '{}'", m_ID, m_Component));

		target->pComponent->Add(target->Entity);

		if (!m_FieldsJson.empty())
		{
			if (Result<void> result = target->pComponent->FromJson(target->Entity, m_FieldsJson); !result)
			{
				target->pComponent->Remove(target->Entity);
				return result;
			}
		}

		context.Events().Queue(ComponentAdded{m_ID, m_Component});
		return {};
	}

	void AddComponentCommand::Undo(EngineContext& context)
	{
		ComponentRegistry::Find(m_Component)->Remove(context.GetWorld().FindEntity(m_ID));
		context.Events().Queue(ComponentRemoved{m_ID, m_Component});
	}

	RemoveComponentCommand::RemoveComponentCommand(PolyID id, std::string_view component)
	    : m_ID(id)
	    , m_Component(component)
	{}

	Result<void> RemoveComponentCommand::Execute(EngineContext& context)
	{
		const Result<Target> target = FindTarget(context, m_ID, m_Component);
		if (!target)
			return std::unexpected(target.error());

		if (!target->pComponent->TypeInfo.Removable)
			return MakeError(ErrorCode::ComponentNotRemovable, fmt::format("Component '{}' cannot be removed from entities", m_Component));

		if (!target->pComponent->Has(target->Entity))
			return MakeError(ErrorCode::ComponentNotPresent, fmt::format("Entity {} does not have component '{}'", m_ID, m_Component));

		m_RemovedJson = target->pComponent->ToJson(target->Entity);
		target->pComponent->Remove(target->Entity);

		context.Events().Queue(ComponentRemoved{m_ID, m_Component});
		return {};
	}

	void RemoveComponentCommand::Undo(EngineContext& context)
	{
		const ComponentDesc* pComponent = ComponentRegistry::Find(m_Component);
		Entity               entity     = context.GetWorld().FindEntity(m_ID);

		pComponent->Add(entity);
		if (Result<void> result = pComponent->FromJson(entity, m_RemovedJson); !result)
			POLY_CORE_WARN("Component '{}' of entity {} could not be fully restored:\n{}", m_Component, m_ID, result.error().Message);

		context.Events().Queue(ComponentAdded{m_ID, m_Component});
	}

	SetFieldCommand::SetFieldCommand(PolyID id, std::string_view component, std::string_view field, std::string_view valueJson)
	    : m_ID(id)
	    , m_Component(component)
	    , m_Field(field)
	    , m_ValueJson(valueJson)
	{}

	Result<void> SetFieldCommand::Execute(EngineContext& context)
	{
		const Result<Target> target = FindTarget(context, m_ID, m_Component);
		if (!target)
			return std::unexpected(target.error());

		if (!target->pComponent->Has(target->Entity))
			return MakeError(ErrorCode::ComponentNotPresent, fmt::format("Entity {} does not have component '{}'", m_ID, m_Component));

		const std::vector<FieldInfo>& fields = target->pComponent->TypeInfo.Fields;
		if (std::ranges::find(fields, m_Field, &FieldInfo::Name) == fields.end())
		{
			std::string names;
			for (const FieldInfo& field : fields)
				names += (names.empty() ? "" : ", ") + field.Name;

			return MakeError(ErrorCode::FieldNotFound, fmt::format("Component '{}' has no field '{}'. Available: {}", m_Component, m_Field, names));
		}

		Result<std::string> oldValue = Json::GetMember(target->pComponent->ToJson(target->Entity), m_Field);
		if (!oldValue)
			return std::unexpected(oldValue.error());

		if (Result<void> result = target->pComponent->FromJson(target->Entity, Json::MakeObject(m_Field, m_ValueJson)); !result)
			return result;

		m_OldValueJson = std::move(*oldValue);

		context.Events().Queue(FieldChanged{m_ID, m_Component, m_Field});
		return {};
	}

	void SetFieldCommand::Undo(EngineContext& context)
	{
		const ComponentDesc* pComponent = ComponentRegistry::Find(m_Component);
		Entity               entity     = context.GetWorld().FindEntity(m_ID);

		if (Result<void> result = pComponent->FromJson(entity, Json::MakeObject(m_Field, m_OldValueJson)); !result)
			POLY_CORE_WARN("Field '{}.{}' of entity {} could not be restored:\n{}", m_Component, m_Field, m_ID, result.error().Message);

		context.Events().Queue(FieldChanged{m_ID, m_Component, m_Field});
	}

	bool SetFieldCommand::TryMerge(const Command& next)
	{
		const SetFieldCommand* pNext = dynamic_cast<const SetFieldCommand*>(&next);
		if (!pNext || pNext->m_ID != m_ID || pNext->m_Component != m_Component || pNext->m_Field != m_Field)
			return false;

		// The old value stays the one from before the first change
		m_ValueJson = pNext->m_ValueJson;
		return true;
	}
} // namespace Poly::API
