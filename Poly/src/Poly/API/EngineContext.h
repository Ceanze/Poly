#pragma once

#include "Poly/API/CommandHistory.h"
#include "Poly/API/Events.h"
#include "Poly/API/Services/ComponentService.h"
#include "Poly/API/Services/EntityService.h"
#include "Poly/API/Services/WorldService.h"
#include "Poly/World/World.h"

namespace Poly::API
{
	/*
	 * Entry point of the engine API. Owns the world and everything needed to change it in a controlled way:
	 * queries return copies, changes are undoable commands and every change is reported as an event.
	 *
	 * Editors and other tools only use the services, they never touch the World directly. The World is exposed
	 * for the application owning the context, to add systems to it and to submit it for rendering.
	 *
	 * Not thread safe, everything must be called from the main thread.
	 *
	 * Usage:
	 * @code
	 * API::EngineContext context;
	 * context.GetWorld().AddSystem<RenderSystem>(World::Phase::PostUpdate, catalog);
	 *
	 * Result<PolyID> entity = context.Entities().Create(PolyID::None(), "Light");
	 * context.Components().SetField(*entity, "Transform", "Translation", "[0, 2, 0]");
	 * context.History().Undo();
	 *
	 * // Every frame
	 * context.Tick();
	 * renderer.Submit({.pWorld = &context.GetWorld()});
	 * @endcode
	 */
	class EngineContext
	{
	public:
		explicit EngineContext(std::string_view worldName = "Untitled");
		~EngineContext() = default;

		CLASS_REMOVE_COPY(EngineContext);
		CLASS_REMOVE_MOVE(EngineContext);

		WorldService&     Worlds() { return m_Worlds; }
		EntityService&    Entities() { return m_Entities; }
		ComponentService& Components() { return m_Components; }
		CommandHistory&   History() { return m_History; }
		EventBus&         Events() { return m_Events; }

		/**
		 * The world itself, only for adding systems and rendering - changes to its entities go through the services
		 */
		World&       GetWorld() { return m_World; }
		const World& GetWorld() const { return m_World; }

		/**
		 * Delivers the events of the changes made since the last tick and updates the world. Call once per frame
		 */
		void Tick();

	private:
		World          m_World;
		EventBus       m_Events;
		CommandHistory m_History;

		WorldService     m_Worlds;
		EntityService    m_Entities;
		ComponentService m_Components;
	};
} // namespace Poly::API
