#include "SyncPlan.h"

namespace Poly
{
	SyncPlan::SyncPlan(std::vector<PassSyncPlan> plans)
	    : m_Plans(std::move(plans))
	{}
} // namespace Poly
