#pragma once

#include <unordered_set>

namespace Poly
{
	struct RGCContext;
	struct CompiledGraph;

	class RGCCompiler
	{
	public:
		RGCCompiler() = default;

		void Execute(RGCContext& ctx);

	private:
		std::unordered_set<uint32> GetMandatoryNodes(RGCContext& ctx);
		std::vector<uint32>        PruneAndSortGraph(RGCContext& ctx, const std::unordered_set<uint32>& mandatoryPasses);
		CompiledGraph              Compile(RGCContext& ctx, std::vector<uint32> sortedNodes);
	};
} // namespace Poly