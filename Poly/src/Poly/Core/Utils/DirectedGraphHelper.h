#pragma once

#include "DirectedGraph.h"
#include "Poly/Core/PolyUtils.h"

#include <stack>

namespace Poly
{
	/**
	 * DirectedGraph implements a graph. DirectedGraphHelper supplies functions to use on the precreated graph
	 */

	class DirectedGraphHelper
	{
	public:
		enum class CreateFlags
		{
			NONE			= FLAG(0),
			REVERSE			= FLAG(1),
			IGNORE_VISITED	= FLAG(2)
		};

		enum class SortFlags
		{
			NONE			= FLAG(0),
			DEPTH_FIRST		= FLAG(1)
			// BREADTH_FIRST	= FLAG(2)
		};


		static const constexpr uint32 INVALID_ID = UINT32_MAX;

	public:
		DirectedGraphHelper(Ref<DirectedGraph> pGraph, uint32 rootNode, CreateFlags flags);

		/**
		 * Travese the graph with the flags given when constructed
		 * @return id of next node, DirectedGraphHelper::INVALID_ID if end is reached
		 */
		uint32 Traverse(bool checkCycle = false);

		/**
		 * Does a topological sort of the graph
		 * @return All connected nodes in the sorted order
		 */
		std::vector<uint32> TopologySort(SortFlags flags = SortFlags::DEPTH_FIRST);

		/**
		 * Checks if the graph is cyclic - i.e. a node can connect to itself creating a path  - NOTE: Resets traverse
		 * @param node - node to check cycle on
		 * @return true if cyclic
		 */

		bool IsCyclic(uint32 node);

		/**
		 * Checks if there exist a path between two nodes - NOTE: Resets traverse
		 * @param from - node to start from
		 * @param to - node to go to
		 * @return true if path exist
		 */
		bool HasPath(uint32 from, uint32 to);

		/**
		 * Resets traverse and visted data
		 */
		void Reset(uint32 rootNode);

		void SetRootNode(uint32 rootNode) { if (m_pGraph->NodeExists(rootNode)) m_RootNode = rootNode; }

		uint32 GetRootNode() const { return m_RootNode; }

	private:
		void VisitNode(uint32 node, std::stack<uint32>& sortedNodes);

		Ref<DirectedGraph>	m_pGraph		= nullptr;
		uint32				m_RootNode		= 0;
		CreateFlags			m_Flags			= CreateFlags::NONE;
		std::vector<bool>	m_Visited;
		std::stack<uint32>	m_TraverseStack;
	};

	ENABLE_BITMASK_OPERATORS(Poly::DirectedGraphHelper::CreateFlags);
	ENABLE_BITMASK_OPERATORS(Poly::DirectedGraphHelper::SortFlags);
}