#include "RenderGraph.h"

#include "Compiler/RenderGraphCompilerNew.h"
#include "ExternalPass.h"
#include "Platform/API/Buffer.h"
#include "Platform/API/Sampler.h"
#include "Poly/Core/RenderAPI.h"
#include "Poly/Core/Utils/DirectedGraph.h"
#include "Poly/Core/Window.h"
#include "Poly/Poly/Format.h"
#include "Poly/Rendering/RenderGraph/EdgeData.h"
#include "Poly/Rendering/RenderGraph/Resource.h"
#include "Poly/Rendering/RenderGraph/ResourceGroup.h"
#include "polypch.h"
#include "RenderGraphCompiler.h"
#include "RenderGraphProgram.h"
#include "RenderPass.h"

namespace Poly
{
	RenderGraph::RenderGraph(std::string name)
	    : m_Name(std::move(name))
	{
		m_pGraph = DirectedGraph::Create();

		auto pExternalPass             = CreateRef<ExternalPass>();
		m_ExternalPassNodeID           = m_pGraph->AddNode();
		m_NameToNodeIndex[PassID("$")] = m_ExternalPassNodeID;
		m_Passes[m_ExternalPassNodeID] = pExternalPass;

		m_DefaultParams = {
		    .TextureWidth        = 1280,
		    .TextureHeight       = 720,
		    .MaxBackbufferCount  = 3,
		    .pSampler            = Sampler::GetDefaultLinearSampler(),
		    .Format              = EFormat::R8G8B8A8_UNORM, // Currently not used
		    .EnableDebugTextures = true};
	}

	Ref<RenderGraph> RenderGraph::Create(std::string name)
	{
		return CreateRef<RenderGraph>(std::move(name));
	}

	RenderGraph RenderGraph::Clone() const
	{
		RenderGraph copy(m_Name);
		copy.m_pGraph             = m_pGraph->Clone();
		copy.m_NameToNodeIndex    = m_NameToNodeIndex;
		copy.m_Passes             = m_Passes;
		copy.m_Edges              = m_Edges;
		copy.m_Outputs            = m_Outputs;
		copy.m_MandatoryPasses    = m_MandatoryPasses;
		copy.m_ExternalPassNodeID = m_ExternalPassNodeID;
		copy.m_DefaultParams      = m_DefaultParams;
		return copy;
	}

	Ref<RenderGraphProgram> RenderGraph::Compile()
	{
		const bool useNewCompiler = true;
		if (useNewCompiler)
		{
			RenderGraphCompilerNew  compiler;
			Ref<RenderGraphProgram> program1 = compiler.Compile(this, m_DefaultParams);
			program1->Init();
			return program1;
		}

		Ref<RenderGraphCompiler> compiler2 = RenderGraphCompiler::Create();
		Ref<RenderGraphProgram>  program2  = compiler2->Compile(this, m_DefaultParams);
		program2->Init();
		return program2;
	}

	bool RenderGraph::AddPass(const Ref<Pass>& pPass, const PassID& passID)
	{
		POLY_VALIDATE(pPass, "Added pass cannot be nullptr");

		if (m_NameToNodeIndex.contains(passID))
		{
			POLY_CORE_WARN("Pass with ID {} has already been added to the render graph, ignoring call", passID.GetName());
			return false;
		}

		uint32 index              = m_pGraph->AddNode();
		m_NameToNodeIndex[passID] = index;
		m_Passes[index]           = pPass;

		pPass->p_Name = passID.GetName();

		return true;
	}

	bool RenderGraph::Removepass(const PassID& passID)
	{
		if (!m_NameToNodeIndex.contains(passID))
		{
			POLY_CORE_WARN("Tried to remove pass with ID {}, but that pass has not been added previously or already been removed", passID.GetName());
			return false;
		}

		// When removing a pass all links that are connected to it also needs to be removed
		// If the pass is marked for output it will also have to be removed

		POLY_VALIDATE(false, "RemovePass is not yet implemented");
		return false;
	}

	bool RenderGraph::AddLink(const PassResID& src, const PassResID& dst)
	{
		const auto srcItr = m_NameToNodeIndex.find(src.GetPass());
		const auto dstItr = m_NameToNodeIndex.find(dst.GetPass());

		if (srcItr == m_NameToNodeIndex.end())
		{
			POLY_CORE_WARN("Source pass with name {} could not be found in the graph when adding link", src.GetPass().GetName());
			return false;
		}

		if (dstItr == m_NameToNodeIndex.end())
		{
			POLY_CORE_WARN("Destination pass with name {} could not be found in the graph when adding link", dst.GetPass().GetName());
			return false;
		}

		// Make sure it is a data-data connection
		if (!src.GetResource().IsValid() || !dst.GetResource().IsValid())
		{
			POLY_CORE_WARN("Cannot link {} with {}, dependency type does not match (both must be data-data dependency)", src.GetFullName(), dst.GetFullName());
			return false;
		}

		uint32 index   = m_pGraph->AddEdge(srcItr->second, dstItr->second);
		m_Edges[index] = {src, dst};

		return true;
	}

	bool RenderGraph::AddLink(const ResID& src, const PassResID& dst)
	{
		auto* pExtPass = static_cast<ExternalPass*>(m_Passes[m_ExternalPassNodeID].get());
		if (!pExtPass->HasResource(src))
		{
			POLY_CORE_WARN("Tried to link external resource {} to {} but the resource has not been added to the graph", src.GetNameAsExternal(), dst.GetFullName());
			return false;
		}

		const auto dstItr = m_NameToNodeIndex.find(dst.GetPass());
		if (dstItr == m_NameToNodeIndex.end())
		{
			POLY_CORE_WARN("Pass with name {} could not be found in the graph when adding link for external resource", dst.GetPass().GetName());
			return false;
		}

		uint32 edgeIndex   = m_pGraph->AddEdge(m_ExternalPassNodeID, dstItr->second);
		m_Edges[edgeIndex] = {src, dst};

		return true;
	}

	bool RenderGraph::AddLink(const PassID& src, const PassID& dst)
	{
		const auto srcItr = m_NameToNodeIndex.find(src);
		const auto dstItr = m_NameToNodeIndex.find(dst);

		if (srcItr == m_NameToNodeIndex.end())
		{
			POLY_CORE_WARN("Source pass with name {} could not be found in the graph when adding link", src.GetName());
			return false;
		}

		if (dstItr == m_NameToNodeIndex.end())
		{
			POLY_CORE_WARN("Destination pass with name {} could not be found in the graph when adding link", dst.GetName());
			return false;
		}

		uint32 edgeIndex   = m_pGraph->AddEdge(srcItr->second, dstItr->second);
		m_Edges[edgeIndex] = {src, dst};

		return true;
	}

	bool RenderGraph::RemoveLink(const PassResID& src, const PassResID& dst)
	{
		const auto srcItr = m_NameToNodeIndex.find(src.GetPass());
		const auto dstItr = m_NameToNodeIndex.find(dst.GetPass());

		if (srcItr == m_NameToNodeIndex.end())
		{
			POLY_CORE_WARN("Source pass with name {} could not be found in the graph when adding link", src.GetPass().GetName());
			return false;
		}

		if (dstItr == m_NameToNodeIndex.end())
		{
			POLY_CORE_WARN("Destination pass with name {} could not be found in the graph when adding link", dst.GetPass().GetName());
			return false;
		}

		const std::vector<uint32>& outgoingEdges = m_pGraph->GetNode(srcItr->second)->GetOutgoingEdges();
		for (uint32 edgeID : outgoingEdges)
		{
			const EdgeData& edge = m_Edges[edgeID];

			if (!edge.IsDataDependency())
				continue;

			if (edge.GetSrcPassRes() == src && edge.GetDstPassRes() == dst)
			{
				m_Edges.erase(edgeID);
				m_pGraph->RemoveEdge(edgeID);
				return true;
			}
		}

		POLY_CORE_WARN("Render pass and resource {} does not connect with {}, no link was removed", src.GetFullName(), dst.GetFullName());
		return false;
	}

	bool RenderGraph::RemoveLink(const ResID& src, const PassID& dst)
	{
		auto* pExtPass = static_cast<ExternalPass*>(m_Passes[m_ExternalPassNodeID].get());
		if (!pExtPass->HasResource(src))
		{
			POLY_CORE_WARN("Tried to remove link of external resource {} to {} but the resource has not been added to the graph", src.GetNameAsExternal(), dst.GetName());
			return false;
		}

		const auto dstItr = m_NameToNodeIndex.find(dst);
		if (dstItr == m_NameToNodeIndex.end())
		{
			POLY_CORE_WARN("Pass with name {} could not be found in the graph when removing link for external resource", dst.GetName());
			return false;
		}

		const std::vector<uint32>& outgoingEdges = m_pGraph->GetNode(m_ExternalPassNodeID)->GetOutgoingEdges();
		for (auto edgeID : outgoingEdges)
		{
			const auto& edge = m_Edges[edgeID];

			if (!edge.IsExternalResourceToPassResource())
				continue;

			if (edge.GetSrcExternalResource() == src && edge.GetDstPass() == dst)
			{
				m_Edges.erase(edgeID);
				m_pGraph->RemoveEdge(edgeID);
				return true;
			}
		}

		POLY_CORE_WARN("External resource {} does not connect with {}, no link was removed", src.GetNameAsExternal(), dst.GetName());
		return false;
	}

	bool RenderGraph::RemoveLink(const PassID& src, const PassID& dst)
	{
		const auto srcItr = m_NameToNodeIndex.find(src);
		const auto dstItr = m_NameToNodeIndex.find(dst);

		if (srcItr == m_NameToNodeIndex.end())
		{
			POLY_CORE_WARN("Source pass with name {} could not be found in the graph when adding link", src.GetName());
			return false;
		}

		if (dstItr == m_NameToNodeIndex.end())
		{
			POLY_CORE_WARN("Destination pass with name {} could not be found in the graph when adding link", dst.GetName());
			return false;
		}

		const std::vector<uint32>& outgoingEdges = m_pGraph->GetNode(srcItr->second)->GetOutgoingEdges();
		for (auto edgeID : outgoingEdges)
		{
			const auto& edge = m_Edges[edgeID];

			if (!edge.IsPassToPass())
				continue;

			if (edge.GetSrcPass() == src && edge.GetDstPass() == dst)
			{
				m_Edges.erase(edgeID);
				m_pGraph->RemoveEdge(edgeID);
				return true;
			}
		}

		POLY_CORE_WARN("Render pass {} does not connect with {}, no link was removed", src.GetName(), dst.GetName());
		return false;
	}

	bool RenderGraph::AddExternalResource(Ref<Resource> pResource, bool autoBindDescriptor)
	{
		if (!pResource)
		{
			POLY_CORE_WARN("Cannot add external resource, resource pointer was nullptr");
			return false;
		}

		const std::string& name     = pResource->GetName();
		auto*              pExtPass = static_cast<ExternalPass*>(m_Passes[m_ExternalPassNodeID].get());

		ResID resID(name);
		if (pExtPass->HasResource(resID))
		{
			POLY_CORE_WARN("External resource {} has already been added, ignoring call", name);
			return false;
		}

		pExtPass->RegisterResource(resID, {pResource, autoBindDescriptor});

		return true;
	}

	bool RenderGraph::AddExternalResource(const ResourceGroup& resourceGroup)
	{
		const std::string& groupName = resourceGroup.GetGroupName();
		auto*              pExtPass  = static_cast<ExternalPass*>(m_Passes[m_ExternalPassNodeID].get());

		const auto& resources = resourceGroup.GetResources();
		for (auto& resource : resources)
		{
			std::string name = Poly::Format("{}:{}", groupName, resource.first);
			ResID       resID(name);
			if (pExtPass->HasResource(resID))
			{
				POLY_CORE_WARN("External resource {} has already been added, ignoring call", name);
				return false;
			}

			pExtPass->RegisterResource(resID, resource.second);
		}

		return true;
	}

	bool RenderGraph::AddExternalResource(const ResID& resID, uint64 size, FBufferUsage bufferUsage, const void* data, bool autoBindDescriptor)
	{
		auto* pExtPass = static_cast<ExternalPass*>(m_Passes[m_ExternalPassNodeID].get());
		if (pExtPass->HasResource(resID))
		{
			POLY_CORE_WARN("External resource {} has already been added, ignoring call", resID.GetName());
			return false;
		}

		BufferDesc desc     = {};
		desc.Size           = size;
		desc.MemUsage       = EMemoryUsage::GPU_ONLY;
		desc.BufferUsage    = bufferUsage | FBufferUsage::COPY_DST;
		Ref<Buffer> pBuffer = RenderAPI::CreateBuffer(&desc);

		Ref<Resource> pResource = Resource::Create(pBuffer, resID.GetName());

		pExtPass->RegisterResource(resID, {pResource, autoBindDescriptor});

		// TODO: Transfer data to buffer using a stagingbuffer if necessary - Should probably have a stagingBufferCache before implementing this
		if (data)
			POLY_CORE_INFO("AddExternalResource does not currently support instant data transfer");

		return true;
	}

	bool RenderGraph::RemoveExternalResource(const ResID& resID)
	{
		auto* pExtPass = static_cast<ExternalPass*>(m_Passes[m_ExternalPassNodeID].get());
		if (!pExtPass->HasResource(resID))
		{
			POLY_CORE_WARN("External resource {} cannot be removed, it is not currently added", resID.GetName());
			return false;
		}

		pExtPass->RemoveResource(resID);

		return true;
	}

	bool RenderGraph::AddMandatoryPass(const PassID& passID)
	{
		const auto itr = m_NameToNodeIndex.find(passID);
		if (itr == m_NameToNodeIndex.end())
		{
			POLY_CORE_WARN("Cannot mark mandatory pass {}, render pass does not exist in graph", passID.GetName());
			return false;
		}

		m_MandatoryPasses.insert(itr->second);
		return true;
	}

	bool RenderGraph::MarkOutput(const PassResID& passResID)
	{
		const auto srcItr = m_NameToNodeIndex.find(passResID.GetPass());
		if (srcItr == m_NameToNodeIndex.end())
		{
			POLY_CORE_WARN("Cannot mark output of {}, render pass does not exist in graph", passResID.GetPass().GetName());
			return false;
		}

		if (!passResID.HasResource())
		{
			POLY_CORE_WARN("MarkOutput requires a render pass and resouce name, but only render pass name of {} was given", passResID.GetPass().GetName());
			return false;
		}

		Output output     = {};
		output.NodeID     = srcItr->second;
		output.ResourceID = passResID.GetResource();
		m_Outputs.insert(output);

		return true;
	}

	bool RenderGraph::UnmarkOutput(const PassResID& passResID)
	{
		const auto srcItr = m_NameToNodeIndex.find(passResID.GetPass());
		if (srcItr == m_NameToNodeIndex.end())
		{
			POLY_CORE_WARN("Cannot unmark output of {}, render pass does not exist in graph", passResID.GetPass().GetName());
			return false;
		}

		if (!passResID.HasResource())
		{
			POLY_CORE_WARN("UnmarkOutput requires a render pass and resouce name, but only render pass name of {} was given", passResID.GetPass().GetName());
			return false;
		}

		Output output     = {};
		output.NodeID     = srcItr->second;
		output.ResourceID = passResID.GetResource();
		m_Outputs.erase(output);

		return true;
	}

	const EdgeData& RenderGraph::GetEdgeData(uint32 edgeID) const
	{
		POLY_VALIDATE(m_Edges.contains(edgeID), "Cannot get edge with ID {}, ID does not exist in graph", edgeID);

		return m_Edges.at(edgeID);
	}

	const Ref<Pass>& RenderGraph::GetPass(uint32 nodeID) const
	{
		POLY_VALIDATE(m_Passes.contains(nodeID), "Cannot get pass with ID {}, ID does not exist in graph", nodeID);

		return m_Passes.at(nodeID);
	}
} // namespace Poly
