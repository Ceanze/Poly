#include "polypch.h"
#include "DirectedGraph.h"

namespace Poly
{
	Ref<DirectedGraph> DirectedGraph::Create()
	{
		return CreateRef<DirectedGraph>();
	}

	uint32 DirectedGraph::AddNode()
	{
		m_Nodes[m_CurrentNodeIndex] = Node();
		return m_CurrentNodeIndex++;
	}

	void DirectedGraph::RemoveNode(uint32 id)
	{
		if (!m_Nodes.contains(id))
		{
			POLY_CORE_WARN("Cannot remove node with ID {}, no node with that ID could be found", id);
			return;
		}

		// Remove connected edges
		Node& node = m_Nodes[id];
		RemoveEdges(node.m_IncommingEdges, true);
		RemoveEdges(node.m_OutgoingEdges, false);

		m_Nodes.erase(id);
	}

	uint32 DirectedGraph::AddEdge(uint32 src, uint32 dst)
	{
		// Check if src and dst exist
		if (!m_Nodes.contains(src) || !m_Nodes.contains(dst))
		{
			POLY_CORE_WARN("Cannot add edge with src {} and dst {}, one or both IDs does not have a corresponding node", src, dst);
			return INVALID_ID;
		}

#ifndef POLY_DIRECTED_GRAPH_ENABLE_LOOPS
		if (src == dst)
		{
			POLY_CORE_ERROR("Standard setting does not allow for loops (same src and dst for an edge) as per the simple directed graph ruleset.\
							To enable loops, define POLY_DIRECTED_GRAPH_ENABLE_LOOPS before header include.\
							src and dst were {}", src);
			return INVALID_ID;
		}
#endif

		m_Edges[m_CurrentEdgeIndex] = Edge(src, dst);
		m_Nodes[src].m_OutgoingEdges.push_back(m_CurrentEdgeIndex);
		m_Nodes[dst].m_IncommingEdges.push_back(m_CurrentEdgeIndex);

		return m_CurrentEdgeIndex++;
	}

	void DirectedGraph::RemoveEdge(uint32 id)
	{
		if (!m_Edges.contains(id))
		{
			POLY_CORE_WARN("Cannot remove edge with ID {}, no edge with that ID could be found", id);
			return;
		}

		Edge& edge = m_Edges[id];
		if (!m_Nodes.contains(edge.m_Src) || !m_Nodes.contains(edge.m_Dst))
		{
			POLY_CORE_WARN("Cannot remove edge with ID {}, one or both nodes did not exist", id);
			return;
		}

		RemoveEdgeFromNode(id, edge.m_Src, true);
		RemoveEdgeFromNode(id, edge.m_Dst, false);

		m_Edges.erase(id);
	}

	void DirectedGraph::RemoveEdges(uint32 srcNode, uint32 dstNode)
	{
		if (!m_Nodes.contains(srcNode) || !m_Nodes.contains(dstNode))
		{
			POLY_CORE_WARN("Cannot remove edge with src {} and dst {}, no edge found that matches those IDs", srcNode, dstNode);
			return;
		}

		std::vector<uint32>& outgoingEdges = m_Nodes[srcNode].m_OutgoingEdges;
		auto it = outgoingEdges.begin();
		while (it != outgoingEdges.end())
		{
			uint32 edgeDst = m_Edges[*it].m_Dst;
			if (edgeDst == dstNode)
				it = outgoingEdges.erase(it);
			else
				it++;
		}
	}

	void DirectedGraph::RemoveEdges(std::vector<uint32>& edgeIDs, bool incommingEdges)
	{
		std::vector<uint32> edgesToRemove;
		for (auto edgeID : edgeIDs)
		{
			Edge& edge = m_Edges[edgeID];
			std::vector<uint32>& neighbourEdgeIDs = incommingEdges ? m_Nodes[edge.m_Src].m_OutgoingEdges : m_Nodes[edge.m_Dst].m_IncommingEdges;
			for (auto neighbourEdgeID : neighbourEdgeIDs)
			{
				// Check if neighbourEdge is connected to node to be removed
				if (neighbourEdgeID == edgeID)
					edgesToRemove.push_back(neighbourEdgeID);
			}
		}

		for (auto id : edgesToRemove)
			RemoveEdge(id);
	}

	void DirectedGraph::RemoveEdgeFromNode(uint32 edgeID, uint32 nodeID, bool removeSrc)
	{
		std::vector<uint32>& neighbourEdgeIDs = removeSrc ? m_Nodes[nodeID].m_OutgoingEdges : m_Nodes[nodeID].m_IncommingEdges;
		for (auto it = neighbourEdgeIDs.begin(); it != neighbourEdgeIDs.end(); it++)
		{
			if (*it == edgeID)
			{
				neighbourEdgeIDs.erase(it);
				return;
			}
		}

		POLY_CORE_WARN("Tried to remove edgeID {} from nodeID {} as an {}, but did not succeed (edgeID or nodeID wrong)", edgeID, nodeID, removeSrc ? "outgoing" : "incomming");
	}
}