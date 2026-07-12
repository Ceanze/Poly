#include "RenderProgram.h"

namespace Poly
{
	RenderProgram::RenderProgram(std::vector<ResolvedPass> sortedPasses, SyncPlan syncPlan)
	    : m_Passes(std::move(sortedPasses))
	    , m_SyncPlan(std::move(syncPlan))
	{}
} // namespace Poly
