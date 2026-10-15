#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Poly
{
	struct VertexAttribute
	{
		uint32  Location = 0;      // Location of the input in the vertex shader
		EFormat Format   = EFormat::UNDEFINED;
		uint32  Offset   = 0;      // Offset from the start of a vertex, in bytes
	};

	// The vertex buffer a pass binds. The backend has a single vertex binding, shared by every attribute
	struct VertexLayout
	{
		uint32                       Stride    = 0; // Size of one vertex, in bytes
		EVertexInputRate             InputRate = EVertexInputRate::VERTEX;
		std::vector<VertexAttribute> Attributes;
	};

	/*
	 * Usage:
	 *
	 *   catalog.GetVertexLayoutRegistry().Register("ImDrawVert", {.Stride     = sizeof(ImDrawVert),
	 *                                                             .Attributes = {{0, EFormat::R32G32_SFLOAT, offsetof(ImDrawVert, pos)}}});
	 */
	class VertexLayoutRegistry
	{
	public:
		/*
		 * Registers a vertex layout. Replaces a layout already registered with the same name.
		 * A layout with attributes but no stride is invalid and not registered.
		 * @param name The name passes reference the layout by.
		 * @param layout The stride, input rate and attributes of the vertex buffer.
		 */
		void Register(std::string_view name, VertexLayout layout);

		const VertexLayout* Get(std::string_view name) const;

		const std::unordered_map<std::string, VertexLayout>& GetAll() const { return m_Layouts; }

	private:
		std::unordered_map<std::string, VertexLayout> m_Layouts;
	};
} // namespace Poly
