#pragma once

#include "Poly/Core/Core.h"

namespace Poly
{
	struct RGCContext;
	class RenderGraphProgram;

	class RGCProgramCreator
	{
	public:
		RGCProgramCreator() = default;

		Ref<RenderGraphProgram> Execute(RGCContext& ctx);
	};
}
