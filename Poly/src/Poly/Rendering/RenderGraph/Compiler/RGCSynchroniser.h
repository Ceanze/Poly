#pragma once

#include "RGCSyncTypes.h"

#include <unordered_set>
#include <vector>

namespace Poly
{
	struct RGCContext;
	struct SyncContext;
	struct SyncPassData;

	class RGCSynchroniser
	{
	public:
		RGCSynchroniser() = default;

		void Execute(RGCContext& ctx);

	private:
		void                      InitialiseResourceStates(RGCContext& ctx, SyncContext& syncCtx);
		void                      SetupPassUsages(RGCContext& ctx, SyncContext& syncCtx);
		std::vector<SyncPassData> BuildSyncPasses(RGCContext& ctx, SyncContext& syncCtx);
		void                      SetOutputLayouts(RGCContext& ctx);
		void                      InsertSyncPasses(RGCContext& ctx, const std::vector<SyncPassData>& syncPasses);
	};
} // namespace Poly
