#include "polypch.h"
#include "RenderPass.h"
#include "RenderGraph.h"
#include "Poly/Core/Window.h"
#include "RenderGraphProgram.h"
#include "RenderGraphCompiler.h"
#include "Poly/Core/RenderAPI.h"
#include "Platform/API/Buffer.h"
#include "Platform/API/Sampler.h"
#include "Poly/Core/Utils/DirectedGraph.h"
#include "Poly/Rendering/RenderGraph/Resource.h"
#include "Poly/Rendering/RenderGraph/ResourceGroup.h"

namespace Poly
{
	RenderGraph::RenderGraph(const std::string& name)
	: m_Name(std::move(name))
	{
		m_pGraph = DirectedGraph::Create();

		Window* window = RenderAPI::GetWindow();
		m_DefaultParams = {
			.TextureWidth		= window->GetWidth(),
			.TextureHeight		= window->GetHeight(),
			.MaxBackbufferCount	= 3,
			.pSampler			= Sampler::GetDefaultLinearSampler()
		};
	}

	Ref<RenderGraph> RenderGraph::Create(const std::string& name)
	{
		return CreateRef<RenderGraph>(name);
	}

	Ref<RenderGraphProgram> RenderGraph::Compile()
	{
		Ref<RenderGraphCompiler> compiler = RenderGraphCompiler::Create();
		Ref<RenderGraphProgram> program = compiler->Compile(this, m_DefaultParams);
		program->Init();
		return program;
	}

	bool RenderGraph::AddPass(const Ref<Pass>& pPass, const std::string& name)
	{
		POLY_VALIDATE(pPass, "Added pass cannot be nullptr");

		if (name.contains('.'))
		{
			POLY_CORE_WARN("Cannot add a pass with name {}, its naming is invalid (contains a dot)", name);
			return false;
		}

		if (m_NameToNodeIndex.contains(name))
		{
			POLY_CORE_WARN("Pass with name {} has already been added to the render graph, ignoring call", name);
			return false;
		}

		uint32 index = m_pGraph->AddNode();
		m_NameToNodeIndex[name] = index;
		m_Passes[index] = pPass;

		pPass->p_Name = name;

		return true;
	}

	bool RenderGraph::Removepass(const std::string& name)
	{
		if (!m_NameToNodeIndex.contains(name))
		{
			POLY_CORE_WARN("Tried to remove pass with name {}, but that pass has not been added previously or already been removed", name);
			return false;
		}

		// When removing a pass all links that are connected to it also needs to be removed
		// If the pass is marked for output it will also have to be removed
	}

	bool RenderGraph::AddLink(const ResourceGUID& src, const ResourceGUID& dst)
	{
		// Check global space first
		if (src.IsExternal())
		{
			if (!m_ExternalResources.contains(src))
			{
				POLY_CORE_WARN("Tried to link external resource {} to {} but the resource has not been added to the graph", src.GetFullName(), dst.GetFullName());
				return false;
			}

			if (!m_NameToNodeIndex.contains(dst.GetPassName()))
			{
				POLY_CORE_WARN("Pass with name {} could not be found in the graph when adding link for external resource", dst.GetPassName());
				return false;
			}

			uint32 nodeIndex = m_NameToNodeIndex[dst.GetPassName()];
			Pass::ExternalResourceData data = { .SrcGUID = src, .DstGUID = dst };
			m_Passes[nodeIndex]->p_ExternalResources.emplace_back(std::move(data));

			return true;
		}

		// If not global space check other passes
		if (!m_NameToNodeIndex.contains(src.GetPassName()) || !m_NameToNodeIndex.contains(dst.GetPassName()))
		{
			POLY_CORE_WARN("Pass with either name {} or {} could not be found in the graph when adding link", src.GetPassName(), dst.GetPassName());
			return false;
		}

		// Make sure it is a data-data connected or an execution-execution connection
		if (src.GetResourceName().empty() && !src.GetResourceName().empty() || !src.GetResourceName().empty() && src.GetResourceName().empty())
		{
			POLY_CORE_WARN("Cannot link {} with {}, dependency type does not match (both must be data-data or exe-exe dep.)", src.GetFullName(), dst.GetFullName());
			return false;
		}


		uint32 index = m_pGraph->AddEdge(m_NameToNodeIndex[src.GetPassName()], m_NameToNodeIndex[dst.GetPassName()]);

		EdgeData edgeData = {};
		edgeData.Src = src;
		edgeData.Dst = dst;
		m_Edges[index] = edgeData;

		return true;
	}

	bool RenderGraph::RemoveLink(const ResourceGUID& src, const ResourceGUID& dst)
	{
		if (!m_NameToNodeIndex.contains(src.GetPassName()) || !m_NameToNodeIndex.contains(dst.GetPassName()))
		{
			POLY_CORE_WARN("Pass with either name {} or {} could not be found in the graph when removing link", src.GetPassName(), dst.GetPassName());
			return false;
		}

		const std::vector<uint32>& outgoingEdges = m_pGraph->GetNode(m_NameToNodeIndex[src.GetPassName()])->GetOutgoingEdges();
		for (auto edgeID : outgoingEdges)
		{
			if (m_Edges[edgeID].Src == src && m_Edges[edgeID].Dst == dst)
			{
				m_Edges.erase(edgeID);
				m_pGraph->RemoveEdge(edgeID);
				return true;
			}
		}

		POLY_CORE_WARN("Render pass and resource {} does not connect with {}, no link was removed", src.GetFullName(), dst.GetFullName());
		return false;
	}

	bool RenderGraph::AddExternalResource(Ref<Resource> pResource, bool autoBindDescriptor)
	{
		if (!pResource)
		{
			POLY_CORE_WARN("Cannot add external resource, resource pointer was nullptr");
			return false;
		}

		const std::string& name = pResource->GetName();

		if (m_ExternalResources.contains(name))
		{
			POLY_CORE_WARN("External resource {} has already been added, ignoring call", name);
			return false;
		}

		m_ExternalResources[name] = { pResource, autoBindDescriptor };

		return true;
	}

	bool RenderGraph::AddExternalResource(const ResourceGroup& resourceGroup)
	{
		const std::string& groupName = resourceGroup.GetGroupName();

		const auto& resources = resourceGroup.GetResources();
		for (auto& resource : resources)
		{
			std::string name = std::format("{}:{}", groupName, resource.first);
			if (m_ExternalResources.contains(name))
			{
				POLY_CORE_WARN("External resource {} has already been added, ignoring call", name);
				return false;
			}

			m_ExternalResources[name] = resource.second;
		}

		return true;
	}

	bool RenderGraph::AddExternalResource(const ResourceGUID& resourceGUID, uint64 size, FBufferUsage bufferUsage, const void* data, bool autoBindDescriptor)
	{
		if (m_ExternalResources.contains(resourceGUID))
		{
			POLY_CORE_WARN("External resource {} has already been added, ignoring call", resourceGUID.GetFullName());
			return false;
		}

		BufferDesc desc = {};
		desc.Size			= size;
		desc.MemUsage		= EMemoryUsage::GPU_ONLY;
		desc.BufferUsage	= bufferUsage | FBufferUsage::COPY_DST;
		Ref<Buffer> pBuffer = RenderAPI::CreateBuffer(&desc);

		Ref<Resource> pResource = Resource::Create(pBuffer, resourceGUID.GetResourceName());

		m_ExternalResources[resourceGUID] = { pResource, autoBindDescriptor };

		// TODO: Transfer data to buffer using a stagingbuffer if necessary - Should probably have a stagingBufferCache before implementing this
		if (data)
			POLY_CORE_INFO("AddExternalResource does not currently support instant data transfer");
	}

	bool RenderGraph::RemoveExternalResource(const ResourceGUID& resourceGUID)
	{
		if (!m_ExternalResources.contains(resourceGUID))
		{
			POLY_CORE_WARN("External resource {} cannot be removed, it is not currently added", resourceGUID.GetFullName());
			return false;
		}

		m_ExternalResources.erase(resourceGUID);

		return true;
	}

	bool RenderGraph::MarkOutput(const ResourceGUID& resourceGUID)
	{
		if (!m_NameToNodeIndex.contains(resourceGUID.GetPassName()))
		{
			POLY_CORE_WARN("Cannot mark output of {}, render pass does not exist in graph", resourceGUID.GetPassName());
			return false;
		}

		if (!resourceGUID.HasResource())
		{
			POLY_CORE_WARN("MarkOutput requires a render pass and resouce name, but only render pass name of {} was given", resourceGUID.GetFullName());
			return false;
		}

		Output output = {};
		output.NodeID = m_NameToNodeIndex[resourceGUID.GetPassName()];
		output.ResourceName = resourceGUID.GetResourceName();
		m_Outputs.insert(output);

		return true;
	}

	bool RenderGraph::UnmarkOutput(const ResourceGUID& resourceGUID)
	{
		if (!m_NameToNodeIndex.contains(resourceGUID.GetPassName()))
		{
			POLY_CORE_WARN("Cannot unmark output of {}, render pass does not exist in graph", resourceGUID.GetPassName());
			return false;
		}

		if (!resourceGUID.HasResource())
		{
			POLY_CORE_WARN("UnmarkOutput requires a render pass and resouce name, but only render pass name of {} was given", resourceGUID.GetFullName());
			return false;
		}

		Output output = {};
		output.NodeID = m_NameToNodeIndex[resourceGUID.GetPassName()];
		output.ResourceName = resourceGUID.GetResourceName();
		m_Outputs.erase(output);

		return true;
	}

	const RenderGraph::EdgeData& RenderGraph::GetEdgeData(uint32 edgeID) const
	{
		if (!m_Edges.contains(edgeID))
		{
			POLY_CORE_WARN("Cannot get edge with ID {}, ID does not exist in graph", edgeID);
			return EdgeData();
		}

		return m_Edges.at(edgeID);
	}

	const Ref<Pass>& RenderGraph::GetPass(uint32 nodeID) const
	{
		if (!m_Passes.contains(nodeID))
		{
			POLY_CORE_WARN("Cannot get pass with ID {}, ID does not exist in graph", nodeID);
			return nullptr;
		}

		return m_Passes.at(nodeID);
	}

	void RenderGraph::SetDefaultParams(RenderGraphDefaultParams* pDefaultParams)
	{
		if (pDefaultParams->TextureHeight)
			m_DefaultParams.TextureHeight = pDefaultParams->TextureHeight;
		if (pDefaultParams->TextureWidth)
			m_DefaultParams.TextureWidth = pDefaultParams->TextureWidth;
		if (pDefaultParams->MaxBackbufferCount)
			m_DefaultParams.MaxBackbufferCount = pDefaultParams->MaxBackbufferCount;
		if (pDefaultParams->pSampler)
			m_DefaultParams.pSampler = pDefaultParams->pSampler;
	}
}