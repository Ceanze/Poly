#pragma once

namespace Poly
{
	struct RGCContext;

	class RGCPassCompiler
	{
	public:
		RGCPassCompiler() = default;

		void Execute(RGCContext& ctx);
	};
} // namespace Poly