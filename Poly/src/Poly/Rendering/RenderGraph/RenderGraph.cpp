#include "polypch.h"
#include "RenderGraph.h"

namespace Poly
{
	RenderGraph::RenderGraph(const std::string& name)
	{
		m_Name = std::move(name);
	}

	Ref<RenderGraph> RenderGraph::Create(const std::string& name)
	{
		return CreateRef<RenderGraph>(name);
	}
}