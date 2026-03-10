#pragma once

namespace Poly
{
	struct RGCContext;
	struct SyncContext;

	class RGCSynchroniser
	{
	public:
		RGCSynchroniser() = default;

		void Execute(RGCContext& ctx);

	private:
		void SetupResourceUsage(RGCContext& ctx, SyncContext& syncCtx);
	};
}