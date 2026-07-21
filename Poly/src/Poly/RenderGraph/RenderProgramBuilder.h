#pragma once

#include "RenderCatalog.h"
#include "RenderProgram.h"

#include <string>
#include <vector>

namespace Poly
{
	class RenderProgramBuilder
	{
	public:
		explicit RenderProgramBuilder(Ref<RenderCatalog> catalog);

		RenderProgramBuilder& AddFeature(std::string_view name);

		Ref<RenderProgram> Build();

	private:
		std::vector<ResolvedPass>    FlattenFeatures();
		std::vector<struct PassNode> BuildDAG(const std::vector<ResolvedPass>& flat) const;
		std::vector<ResolvedPass>    TopoSortALAP(const std::vector<struct PassNode>& nodes,
		                                          const std::vector<ResolvedPass>&    flat) const;
		SyncPlan                     PlanSynchronization(const std::vector<ResolvedPass>& passes) const;

		Ref<RenderCatalog>       m_Catalog;
		std::vector<std::string> m_Features;
	};
} // namespace Poly
