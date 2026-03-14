#pragma once

namespace Poly
{
	class RGCContext;

	class RGCValidator
	{
	public:
		RGCValidator() = default;

		bool Execute(RGCContext& ctx);
	};
}