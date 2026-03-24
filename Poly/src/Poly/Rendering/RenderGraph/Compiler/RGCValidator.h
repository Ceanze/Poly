#pragma once

namespace Poly
{
	struct RGCContext;

	class RGCValidator
	{
	public:
		RGCValidator() = default;

		bool Execute(RGCContext& ctx);
	};
}