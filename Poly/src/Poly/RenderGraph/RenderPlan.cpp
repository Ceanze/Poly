#include "RenderPlan.h"

namespace Poly
{
	RenderPlan::RenderPlan(std::vector<RenderPlanEntry> entries)
	    : m_Entries(std::move(entries))
	{}

	uint32 RenderPlan::FindViewStage(std::string_view name) const
	{
		for (uint32 i = 0; i < m_Entries.size(); i++)
		{
			if (m_Entries[i].Type == RenderPlanEntry::EType::ViewStage && m_Entries[i].Name == name)
				return i;
		}

		return INVALID_ENTRY;
	}
} // namespace Poly
