#pragma once

namespace Poly
{
	struct RGCContext;

	class RGCResourceOutputHandler
	{
	public:
		RGCResourceOutputHandler() = default;

		void Execute(RGCContext& ctx);
	};
}