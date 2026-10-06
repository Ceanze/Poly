#pragma once

namespace Editor
{
	/**
	 * State of the editor itself, as opposed to the state of the world which lives behind the API
	 */
	struct EditorState
	{
		Poly::PolyID Selection = Poly::PolyID::None();
	};
} // namespace Editor
