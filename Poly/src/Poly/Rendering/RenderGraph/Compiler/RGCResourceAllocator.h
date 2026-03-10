#pragma once

namespace Poly
{
	struct RGCContext;

	class RGCResourceAllocator
	{
	public:
		RGCResourceAllocator() = default;

		void Execute(RGCContext& ctx);
	};
}