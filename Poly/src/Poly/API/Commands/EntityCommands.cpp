#include "EntityCommands.h"

#include "Poly/API/EngineContext.h"
#include "Poly/Reflection/ComponentRegistry.h"
#include "Poly/Reflection/JsonHelpers.h"

namespace
{
	void CaptureSubtree(Poly::Entity entity, std::vector<Poly::EntityData>& entities)
	{
		entities.push_back(Poly::WorldSerializer::CaptureEntity(entity));
		for (Poly::Entity child : entity.GetChildren())
			CaptureSubtree(child, entities);
	}
} // namespace

namespace Poly::API
{
	CreateEntityCommand::CreateEntityCommand(PolyID parent, std::string_view name, uint32 siblingIndex)
	    : m_Parent(parent)
	    , m_Name(name)
	    , m_SiblingIndex(siblingIndex)
	{}

	Result<void> CreateEntityCommand::Execute(EngineContext& context)
	{
		World& world = context.GetWorld();

		Entity parent = Entity::None();
		if (m_Parent != PolyID::None())
		{
			parent = world.FindEntity(m_Parent);
			if (!parent.IsValid())
				return MakeError(ErrorCode::EntityNotFound, fmt::format("Cannot create entity, parent {} does not exist", m_Parent));
		}

		// The ID is kept, so a redo creates the same entity as far as everyone else is concerned
		if (m_ID == PolyID::None())
			m_ID = PolyID();

		Entity entity = world.CreateEntity(m_ID);

		if (const ComponentDesc* pName = ComponentRegistry::Find("Name"); pName && !m_Name.empty())
		{
			pName->Add(entity);
			pName->FromJson(entity, Json::MakeObject("Name", Json::Quote(m_Name)));
		}

		if (parent.IsValid())
			entity.SetParent(parent, m_SiblingIndex);

		context.Events().Queue(EntityCreated{m_ID});
		return {};
	}

	void CreateEntityCommand::Undo(EngineContext& context)
	{
		context.GetWorld().DestroyEntity(context.GetWorld().FindEntity(m_ID));
		context.Events().Queue(EntityDestroyed{m_ID});
	}

	DestroyEntityCommand::DestroyEntityCommand(PolyID id)
	    : m_ID(id)
	{}

	Result<void> DestroyEntityCommand::Execute(EngineContext& context)
	{
		World& world = context.GetWorld();

		Entity entity = world.FindEntity(m_ID);
		if (!entity.IsValid())
			return MakeError(ErrorCode::EntityNotFound, fmt::format("Cannot destroy entity, {} does not exist", m_ID));

		m_Entities.clear();
		CaptureSubtree(entity, m_Entities);
		m_SiblingIndex = entity.GetSiblingIndex();

		world.DestroyEntity(entity);

		context.Events().Queue(EntityDestroyed{m_ID});
		return {};
	}

	void DestroyEntityCommand::Undo(EngineContext& context)
	{
		World& world = context.GetWorld();

		// Create all entities first, so parents exist when their children are linked to them
		for (const EntityData& data : m_Entities)
			world.CreateEntity(data.ID);

		for (size_t i = 0; i < m_Entities.size(); i++)
		{
			const EntityData& data   = m_Entities[i];
			Entity            entity = world.FindEntity(data.ID);

			// The root goes back to where it was among its siblings, the descendants are appended in their captured order
			if (Entity parent = world.FindEntity(data.Parent); parent.IsValid())
				entity.SetParent(parent, i == 0 ? m_SiblingIndex : Entity::LAST_SIBLING_INDEX);

			WorldSerializer::ApplyComponents(entity, data);
		}

		context.Events().Queue(EntityCreated{m_ID});
	}

	SetParentCommand::SetParentCommand(PolyID id, PolyID parent, uint32 siblingIndex)
	    : m_ID(id)
	    , m_Parent(parent)
	    , m_SiblingIndex(siblingIndex)
	{}

	Result<void> SetParentCommand::Execute(EngineContext& context)
	{
		World& world = context.GetWorld();

		Entity entity = world.FindEntity(m_ID);
		if (!entity.IsValid())
			return MakeError(ErrorCode::EntityNotFound, fmt::format("Cannot set parent, entity {} does not exist", m_ID));

		Entity parent = Entity::None();
		if (m_Parent != PolyID::None())
		{
			parent = world.FindEntity(m_Parent);
			if (!parent.IsValid())
				return MakeError(ErrorCode::EntityNotFound, fmt::format("Cannot set parent of entity {}, parent {} does not exist", m_ID, m_Parent));

			for (Entity ancestor = parent; ancestor.IsValid(); ancestor = ancestor.GetParent())
			{
				if (ancestor == entity)
					return MakeError(ErrorCode::InvalidParent, fmt::format("Cannot set parent of entity {} to {}, it is the entity itself or one of its descendants", m_ID, m_Parent));
			}
		}

		Entity oldParent  = entity.GetParent();
		m_OldParent       = oldParent.IsValid() ? oldParent.GetPolyID() : PolyID::None();
		m_OldSiblingIndex = entity.GetSiblingIndex();

		entity.SetParent(parent, m_SiblingIndex);
		entity.MarkDirty();

		context.Events().Queue(EntityReparented{m_ID, m_OldParent, m_Parent});
		return {};
	}

	void SetParentCommand::Undo(EngineContext& context)
	{
		World& world = context.GetWorld();

		Entity entity = world.FindEntity(m_ID);
		entity.SetParent(world.FindEntity(m_OldParent), m_OldSiblingIndex);
		entity.MarkDirty();

		context.Events().Queue(EntityReparented{m_ID, m_Parent, m_OldParent});
	}
} // namespace Poly::API
