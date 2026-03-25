#pragma once

#include "Poly/Rendering/RenderGraph/Reflection/PassReflection.h"

namespace Poly
{
	class Pass;

	struct CompiledPass
	{
		Ref<Pass>      pPass;
		PassReflection Reflection;
		uint32         GraphNodeIndex;
		uint32         ExecutionIndex;

		bool operator==(const CompiledPass& other) const { return GraphNodeIndex == other.GraphNodeIndex; }
	};

	struct CompiledGraph
	{
		std::vector<CompiledPass> CompiledPasses;
	};
} // namespace Poly