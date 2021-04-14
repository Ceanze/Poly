#pragma once

#include "Poly/Core/Core.h"

namespace Poly
{
	/**
	 * A directed graph is a graph in which all of the edges that are connecting the nodes also have a direction.
	 * Standard implementation does not allow loops on nodes (connecting an edge to themselves), this is therefore
	 * a "Simple Directed Graph". To enable loops define POLY_DIRECTED_GRAPH_ENABLE_LOOPS before including header.
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

		/**
		 * @return true if node exists in the graph
		 */
		bool NodeExists(uint32 nodeID) const { return m_Nodes.contains(nodeID); }

		/**
		 * @return true if edge exists in the graph
		 */
		bool EdgeExists(uint32 edgeID) const { return m_Edges.contains(edgeID); }

		/**
		 * @return Current node index
		 */
		uint32 CurrentNodeIndex() const { return m_CurrentNodeIndex; }

		/**
		 * @return Current edge index
		 */
		uint32 CurrentEdgeIndex() const { return m_CurrentEdgeIndex; }

		/**
		 * @return Node* for the given ID, nullptr if node does not exist
		 */
		const Node* GetNode(uint32 id) const { return m_Nodes.contains(id) ? &(m_Nodes.at(id)) : nullptr; }

		/**
		 * @return Edge* for the given ID, nullptr if edge does not exist
		 */
		const Edge* GetEdge(uint32 id) const { return m_Edges.contains(id) ? &(m_Edges.at(id)) : nullptr; }

	private:
		void RemoveEdges(std::vector<uint32>& edgeIDs, bool incommingEdges);
		void RemoveEdgeFromNode(uint32 edgeID, uint32 nodeID, bool removeSrc);

		std::unordered_map<uint32, Node> m_Nodes;
		std::unordered_map<uint32, Edge> m_Edges;
		uint32 m_CurrentNodeIndex = 0;
		uint32 m_CurrentEdgeIndex = 0;
	};
}