#pragma once

#include "EditorState.h"

#include <Poly/API/EngineContext.h>

namespace Editor
{
	/*
	 * Shows and edits the components of the selected entity, drawn generically from the fields of each component.
	 */
	class InspectorPanel
	{
	public:
		void Draw(Poly::API::EngineContext& context, EditorState& state);
	};
} // namespace Editor
