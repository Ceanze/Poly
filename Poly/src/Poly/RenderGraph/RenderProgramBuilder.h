#pragma once

#include "RenderCatalog.h"
#include "RenderProgram.h"

#include <memory>
#include <string>
#include <vector>

namespace Poly
{
	class RenderProgramBuilder
	{
	public:
		explicit RenderProgramBuilder(Ref<RenderCatalog> catalog);

		RenderProgramBuilder& AddFeature(std::string_view name);

		std::unique_ptr<RenderProgram> Build();

	private:
		std::vector<ResolvedPass>    FlattenFeatures() const;
		std::vector<struct PassNode> BuildDAG(const std::vector<ResolvedPass>& flat) const;
		std::vector<ResolvedPass>    TopoSortALAP(const std::vector<struct PassNode>& nodes,
		                                          const std::vector<ResolvedPass>&    flat) const;

		Ref<RenderCatalog>       m_Catalog;
		std::vector<std::string> m_Features;
	};
} // namespace Poly
