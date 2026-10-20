#include "VertexLayoutRegistry.h"

namespace Poly
{
	void VertexLayoutRegistry::Register(std::string_view name, VertexLayout layout)
	{
		if (!layout.Attributes.empty() && layout.Stride == 0)
		{
			POLY_CORE_ERROR("Vertex layout '{}' has attributes but no stride, it is not registered", name);
			return;
		}

		auto [it, inserted] = m_Layouts.insert_or_assign(std::string(name), std::move(layout));
		if (!inserted)
			POLY_CORE_WARN("Vertex layout '{}' was already registered, it is replaced", name);
	}

	const VertexLayout* VertexLayoutRegistry::Get(std::string_view name) const
	{
		auto it = m_Layouts.find(name);
		return it != m_Layouts.end() ? &it->second : nullptr;
	}
} // namespace Poly
