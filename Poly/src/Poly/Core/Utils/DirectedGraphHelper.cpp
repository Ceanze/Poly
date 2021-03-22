#include "polypch.h"
#include "DirectedGraphHelper.h"

namespace Poly
{
	DirectedGraphHelper::DirectedGraphHelper(Ref<DirectedGraph> pGraph, uint32 rootNode, CreateFlags flags)
	{
		m_pGraph = pGraph;
		m_RootNode = rootNode;
		m_Flags = flags;

		Reset(rootNode);
	}
	
	uint32 DirectedGraphHelper::Traverse(bool checkCycle)
	{
		if (m_TraverseStack.empty())
			return INVALID_ID;

		uint32 currNode = m_TraverseStack.top();

		m_TraverseStack.pop();

		if (BitsSet(m_Flags, CreateFlags::IGNORE_VISITED))
		{
			// If current node is visited, go thorugh the stack and check if there
			// are any other valid options
			while (m_Visited[currNode])
			{
				if (checkCycle && (currNode == m_RootNode))
					return currNode;

				if (m_TraverseStack.empty())
					return INVALID_ID;

				currNode = m_TraverseStack.top();
				m_TraverseStack.pop();
			}

			m_Visited[currNode] = true;
		}

		bool reversed = BitsSet(m_Flags, CreateFlags::REVERSE);
		const std::vector<uint32>& children = reversed ? m_pGraph->GetNode(currNode)->GetIncommingEdges() : m_pGraph->GetNode(currNode)->GetOutgoingEdges();
		for (const auto& child : children)
		{
			uint32 node = reversed ? m_pGraph->GetEdge(child)->GetSrcNode() : m_pGraph->GetEdge(child)->GetDstNode();
			m_TraverseStack.push(node);
		}

		return currNode;
	}

	std::vector<uint32> DirectedGraphHelper::TopologySort(SortFlags flags)
	{
		// if (BitsSet(flags, SortFlags::DEPTH_FIRST))
		// {
		// 	POLY_CORE_WARN("Only Depth First is currently supported for sorting");
		// 	return;
		// }

		if (BitsSet(flags, SortFlags::NONE))
		{
			POLY_CORE_WARN("SortFlag cannot be \"NONE\" for TopologySort!");
			return {};
		}

		Reset(m_RootNode);

		std::stack<uint32> nodeStack;
		for (uint32 i = 0; i < m_pGraph->CurrentNodeIndex(); i++)
		{
			// Graph does not know which indices that are nodes (could be removed), therefore check that
			if (!m_Visited[i] && m_pGraph->NodeExists(i));
			{
				VisitNode(i, nodeStack);
			}
		}

		// Depth First places the nodes in inverted order, reorder it before returning
		std::vector<uint32> sortedNodes;
		sortedNodes.reserve(nodeStack.size());
		uint32 size = nodeStack.size();
		for (uint32 i = 0; i < size; i++)
		{
			sortedNodes.push_back(nodeStack.top());
			nodeStack.pop();
		}

		return sortedNodes;
	}

	bool DirectedGraphHelper::IsCyclic(uint32 node)
	{
		// A graph is cyclic if there exists a path from and to the same node
		return HasPath(node, node);
	}

	bool DirectedGraphHelper::HasPath(uint32 from, uint32 to)
	{
		Reset(from);

		// First currNode will be the root (from), skip that
		uint32 currNode = Traverse(true);
		currNode = Traverse(true);
		while (currNode != INVALID_ID)
		{
			if (currNode == to)
				return true;
			currNode = Traverse(true);
		}

		return false;
	}

	void DirectedGraphHelper::Reset(uint32 rootNode)
	{
		if (!m_pGraph->NodeExists(rootNode))
		{
			POLY_CORE_ERROR("Could not reset graph - root node with id {} could not be found in graph", rootNode);
			return;
		}

		// Only need to set visted array if we need to ignore previous nodes
		if (BitsSet(m_Flags, CreateFlags::IGNORE_VISITED))
		{
			m_Visited.assign(m_pGraph->CurrentNodeIndex(), false);
		}

		while (!m_TraverseStack.empty())
			m_TraverseStack.pop();
		m_RootNode = rootNode;
		m_TraverseStack.push(rootNode);
	}

	void DirectedGraphHelper::VisitNode(uint32 node, std::stack<uint32>& nodeStack)
	{
		if (m_Visited[node])
			return;

		m_Visited[node] = true;

		const std::vector<uint32> outgoingEdges = m_pGraph->GetNode(node)->GetOutgoingEdges();
		for (const auto& edgeID : outgoingEdges)
		{
			VisitNode(m_pGraph->GetEdge(edgeID)->GetDstNode(), nodeStack);
		}

		nodeStack.push(node);
	}
}