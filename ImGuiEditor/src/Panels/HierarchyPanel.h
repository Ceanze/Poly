#pragma once

#include "EditorState.h"

#include <Poly/API/EngineContext.h>

namespace Editor
{
	/*
	 * Tree of all entities of the world. Select by clicking, reparent by dragging an entity onto another
	 * and create or destroy entities from the context menus.
	 */
	class HierarchyPanel
	{
	public:
		void Draw(Poly::API::EngineContext& context, EditorState& state);

	private:
		enum class EAction
		{
			None,
			Create,
			Destroy,
			SetParent,
		};

		void DrawEntity(Poly::API::EngineContext& context, EditorState& state, Poly::PolyID id);

		// Changes are applied after the tree has been drawn, so the tree never changes while it is being walked
		EAction      m_Action = EAction::None;
		Poly::PolyID m_Target = Poly::PolyID::None();
		Poly::PolyID m_Parent = Poly::PolyID::None();
	};
} // namespace Editor
