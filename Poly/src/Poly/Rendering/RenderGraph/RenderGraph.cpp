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

	void RenderGraph::Compile()
	{

	}

	bool RenderGraph::AddPass(const Ref<RenderPass>& pPass, const std::string& name)
	{
		return false;
	}

	bool RenderGraph::Removepass(const std::string& name)
	{
		return false;
	}

	bool RenderGraph::AddLink(const std::string& src, const std::string& dst)
	{
		return false;
	}

	bool RenderGraph::RemoveLink(const std::string& src, const std::string& dst)
	{
		return false;
	}

	bool RenderGraph::AddInputResource(const std::string& name, Ref<Resource> pResource)
	{
		return false;
	}

	bool RenderGraph::MarkOutput(const std::string& name)
	{
		return false;
	}
}