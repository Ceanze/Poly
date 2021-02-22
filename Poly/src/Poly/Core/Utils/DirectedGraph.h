#pragma once

#include "Poly/Core/Core.h"

namespace Poly
{
	/**
	 * A directed graph is a graph in which all of the edges that are connecting the nodes also have a direction.
	 */

	class DirectedGraph
	{
	public:
		static const constexpr uint32 INVALID_ID = UINT32_MAX;

		class Node
		{
		public:
			Node() = default;
			~Node() = default;

			const std::vector<uint32>& GetIncommingEdges() const { return m_IncommingEdges; }
			const std::vector<uint32>& GetOutgoingEdges() const { return m_OutgoingEdges; }

		private:
			friend DirectedGraph;
			std::vector<uint32> m_IncommingEdges;
			std::vector<uint32> m_OutgoingEdges;
		};

		class Edge
		{
		public:
			Edge() = default;
			Edge(uint32 src, uint32 dst) : m_Src(src), m_Dst(dst) {}
			~Edge() = default;

			uint32 GetSrcNode() const { return m_Src; }
			uint32 GetDstNode() const { return m_Dst; }

		private:
			friend DirectedGraph;
			uint32 m_Src = INVALID_ID;
			uint32 m_Dst = INVALID_ID;
		};

	public:

		DirectedGraph() = default;
		~DirectedGraph() = default;

		static Ref<DirectedGraph> Create();

		/**
		 * Add a node to the graph
		 * @return unique ID for the node
		 */
		uint32 AddNode();

		/**
		 * Remove a node from the graph, this will also remove connected edges
		 */
		void RemoveNode(uint32 id);

		/**
		 * Add a connecting edge between two nodes using their indices
		 * @param src - index of source pass
		 * @param dst - index of destination pass
		 * @return unique ID for the edge
		 */
		uint32 AddEdge(uint32 src, uint32 dst);

		/**
		 * Remove an edge using its ID
		 * @param id - ID of the edge
		 */
		void RemoveEdge(uint32 id);

		/**
		 * Remove an edge using the src and dst nodes
		 * @param src - index of source pass
		 * @param dst - index of destination pass
		 */
		void RemoveEdges(uint32 srcNode, uint32 dstNode);

	private:
		void RemoveEdges(std::vector<uint32>& edgeIDs, bool incommingEdges);
		void RemoveEdgeFromNode(uint32 edgeID, uint32 nodeID, bool removeSrc);

		std::unordered_map<uint32, Node> m_Nodes;
		std::unordered_map<uint32, Edge> m_Edges;
		uint32 m_CurrentNodeIndex = 0;
		uint32 m_CurrentEdgeIndex = 0;
	};
}