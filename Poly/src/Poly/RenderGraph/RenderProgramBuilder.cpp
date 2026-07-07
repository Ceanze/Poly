#include "RenderProgramBuilder.h"

#include "Feature/FeatureDeclaration.h"
#include "Feature/FeaturePort.h"
#include "Pass/PassDeclaration.h"

#include <algorithm>
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace Poly
{
	struct PassNode
	{
		size_t              flatIndex;
		std::vector<size_t> dependsOn;
		bool                mandatory = false;
	};

	RenderProgramBuilder::RenderProgramBuilder(Ref<RenderCatalog> catalog)
	    : m_Catalog(std::move(catalog))
	{}

	RenderProgramBuilder& RenderProgramBuilder::AddFeature(std::string_view name)
	{
		if (!m_Catalog->GetFeatureRegistry().Get(name))
		{
			POLY_CORE_ERROR("Feature '{}' not found in render catalog.", name);
			return *this;
		}

		m_Features.emplace_back(name);
		return *this;
	}

	// Phase 1: Expand features into an ordered flat list of passes with resolved port names.
	// Import/export resource names are scoped per feature instance ("featureName#N.resName")
	// so that two uses of the same feature don't share internal resource connections.
	// Semantic ports (EFeaturePort) and global mappings are intentionally left unscoped.
	std::vector<ResolvedPass> RenderProgramBuilder::FlattenFeatures() const
	{
		std::vector<ResolvedPass>               flat;
		std::unordered_map<std::string, size_t> instanceCount;

		for (const auto& featureName : m_Features)
		{
			const FeatureDeclaration* feature = m_Catalog->GetFeatureRegistry().Get(featureName);
			if (!feature)
			{
				POLY_CORE_ERROR("Feature '{}' not found during compilation.", featureName);
				continue;
			}

			const size_t      instanceIdx = instanceCount[featureName]++;
			const std::string scope       = featureName + "#" + std::to_string(instanceIdx) + ".";

			for (const auto& passName : feature->GetPasses())
			{
				const PassDeclaration* pass = m_Catalog->GetPassRegistry().Get(passName);
				if (!pass)
				{
					POLY_CORE_ERROR("Pass '{}' referenced by feature '{}' not found during compilation.",
					                passName, featureName);
					continue;
				}

				ResolvedPass resolved;
				resolved.Name = passName;

				for (const auto& [port, shaderName] : pass->GetResourceMappings())
					resolved.Ports.push_back({shaderName, std::string(ToSemanticName(port)), /*IsWrite=*/true});

				for (const auto& [globalName, shaderName] : pass->GetGlobalMappings())
					resolved.Ports.push_back({shaderName, globalName, /*IsWrite=*/false});

				for (const auto& [resName, shaderName] : pass->GetImportedResources())
					resolved.Ports.push_back({shaderName, scope + resName, /*IsWrite=*/false});

				for (const auto& [resName, shaderName] : pass->GetExportedResources())
					resolved.Ports.push_back({shaderName, scope + resName, /*IsWrite=*/true});

				flat.push_back(std::move(resolved));
			}
		}

		return flat;
	}

	// Phase 2: Build a culled dependency graph.
	// Passes that don't contribute to the final $Color output or are marked non-mandatory are excluded.
	std::vector<PassNode> RenderProgramBuilder::BuildDAG(const std::vector<ResolvedPass>& flat) const
	{
		const size_t          n = flat.size();
		std::vector<PassNode> nodes(n);
		for (size_t i = 0; i < n; ++i)
			nodes[i].flatIndex = i;

		// Seed: the last pass that writes $Color is the mandatory sink
		for (int i = static_cast<int>(n) - 1; i >= 0; --i)
		{
			for (const auto& port : flat[i].Ports)
			{
				if (port.IsWrite && port.ResolvedName == "$Color")
				{
					nodes[i].mandatory = true;
					break;
				}
			}

			if (nodes[i].mandatory)
				break;
		}

		// availableInputs: resolvedName -> index of the mandatory pass that reads it
		std::unordered_map<std::string, size_t> availableInputs;

		// Reverse scan: pull in producers of resources that mandatory passes need
		for (int i = static_cast<int>(n) - 1; i >= 0; --i)
		{
			const auto& pass = flat[i];

			// Check if any of this pass's writes satisfy a pending read
			if (!nodes[i].mandatory)
			{
				for (const auto& port : pass.Ports)
				{
					if (port.IsWrite && availableInputs.count(port.ResolvedName))
					{
						nodes[i].mandatory = true;
						break;
					}
				}
			}

			if (!nodes[i].mandatory)
				continue;

			// Wire writes → consuming passes as dependencies.
			// A write without a clear is an implicit passthrough (read-then-write), so it
			// also appears in availableInputs and will pull in whichever earlier pass produced it.
			for (const auto& port : pass.Ports)
			{
				if (!port.IsWrite)
					continue;

				auto it = availableInputs.find(port.ResolvedName);
				if (it != availableInputs.end())
				{
					nodes[it->second].dependsOn.push_back((size_t)i);
					availableInputs.erase(it);
				}
			}

			// Register all ports as pending: explicit reads and write-passthroughs.
			// An earlier pass that writes the same resource name will satisfy the passthrough.
			for (const auto& port : pass.Ports)
				availableInputs.emplace(port.ResolvedName, (size_t)i);
		}

		return nodes;
	}

	// Phase 3: Topological sort, scheduled as-late-as-possible (ALAP).
	// Passes with deeper dependency chains are scheduled first so resources
	// have the shortest possible lifetime.
	std::vector<ResolvedPass> RenderProgramBuilder::TopoSortALAP(const std::vector<PassNode>&     nodes,
	                                                             const std::vector<ResolvedPass>& flat) const
	{
		const size_t n = nodes.size();

		// Memoized depth: number of edges on the longest path from this node to a leaf
		std::vector<int>           depth(n, -1);
		std::function<int(size_t)> computeDepth = [&](size_t idx) -> int {
			if (depth[idx] >= 0)
				return depth[idx];

			if (nodes[idx].dependsOn.empty())
				return depth[idx] = 0;

			int maxDep = 0;
			for (size_t dep : nodes[idx].dependsOn)
				maxDep = std::max(maxDep, computeDepth(dep));

			return depth[idx] = 1 + maxDep;
		};

		for (size_t i = 0; i < n; ++i)
			if (nodes[i].mandatory)
				computeDepth(i);

		// Sinks: mandatory nodes that no other mandatory node depends on
		std::unordered_set<size_t> hasDependents;
		for (size_t i = 0; i < n; ++i)
		{
			if (!nodes[i].mandatory)
				continue;

			for (size_t dep : nodes[i].dependsOn)
				hasDependents.insert(dep);
		}

		std::vector<ResolvedPass>  result;
		std::unordered_set<size_t> visited;

		std::function<void(size_t)> visit = [&](size_t idx) {
			if (visited.count(idx))
				return;

			visited.insert(idx);
			// Deepest dependency chains first → latest possible scheduling
			auto deps = nodes[idx].dependsOn;
			std::sort(deps.begin(), deps.end(), [&](size_t a, size_t b) {
				return depth[a] > depth[b];
			});

			for (size_t dep : deps)
				visit(dep);

			result.push_back(flat[idx]);
		};

		for (size_t i = 0; i < n; ++i)
			if (nodes[i].mandatory && !hasDependents.count(i))
				visit(i);

		return result;
	}

	std::unique_ptr<RenderProgram> RenderProgramBuilder::Build()
	{
		auto flat   = FlattenFeatures();
		auto nodes  = BuildDAG(flat);
		auto sorted = TopoSortALAP(nodes, flat);
		return std::make_unique<RenderProgram>(std::move(sorted));
	}
} // namespace Poly
