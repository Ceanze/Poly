#pragma once

#include <queue>
#include <stack>

namespace Poly
{
	typedef uint32 NodeIndex;

	template<typename T>
	class HierarchyTree
	{
	public:
		struct Node
		{
			NodeIndex ParentIndex = UINT32_MAX;
			std::vector<NodeIndex> ChildrenIndices;
			T Data;
		};

		enum class TraverseMode
		{
			NONE			= 0,
			BREADTH_FIRST	= 1,
			DEPTH_FIRST		= 2,
		};

	public:
		HierarchyTree() : m_TraverseMode(TraverseMode::DEPTH_FIRST);
		~HierarchyTree();

		static Ref<HierarchyTree> Create() { return CreateRef<HierarchyTree>(); }

		/**
		 * Adds a new node to the tree.
		 * NOTE: If first node added then added node is added as root node
		 * @param node - node to add
		 * @return NodeIndex of the added node
		 */
		NodeIndex AddNode(Node node);

		/**
		 * Adds a new node to the tree.
		 * NOTE: If first node added then added node is added as root node
		 * @param parentIndex - index of the parent - must be a valid node unless root (first node) being added
		 * @param childrenIndices - list of children by their indices, empty if none
		 * @param data - data of the node
		 * @return NodeIndex of the added node
		 */
		NodeIndex CreateNode(uint32 parentIndex, std::vector<uint32> childrenIndices, T data);

		Node* GetNode(NodeIndex index);

		void AddChild(NodeIndex parentNode, NodeIndex childNode);

		/**
		 * Traverses the tree, returning the data for the current node.
		 * First traverse returns the root node.
		 * Order of traverse depends on if mode is Depth or Breadth first search
		 * @return T data for current visited node
		 */
		T* TraverseNext();

		void ResetTraverse();

		void SetTraverseMode(TraverseMode mode);

	private:
		std::vector<Node> m_Nodes;
		NodeIndex m_CurrentNodeIndex = UINT32_MAX;
		std::queue<uint32> m_TraveseQueue;	// TODO: Template the queue and stack to only have one per tree
		std::stack<uint32> m_TraverseStack;
		TraverseMode m_TraverseMode;
	};

	template<typename T>
	NodeIndex HierarchyTree<T>::AddNode(Node node)
	{
		NodeIndex index = m_Nodes.size();
		m_Nodes.push_back(node);
		return index;
	}

	template<typename T>
	NodeIndex HierarchyTree<T>::CreateNode(uint32 parentIndex, std::vector<uint32> childrenIndices, T data)
	{
		NodeIndex index = m_Nodes.size();
		m_Nodes.push_back({ .ParentIndex = parentIndex, .ChildrenIndices = childrenIndices, .Data = data });
		return index;
	}

	template<typename T>
	HierarchyTree<T>::Node* HierarchyTree<T>::GetNode(NodeIndex index)
	{
		if (index >= m_Nodes.size())
		{
			POLY_CORE_WARN("Cannot get node {}, index is out of range", index);
			return nullptr;
		}

		return &m_Nodes[index];
	}

	template<typename T>
	void HierarchyTree<T>::AddChild(NodeIndex parentNode, NodeIndex childNode)
	{
		Node* pParent = GetNode(parentNode);

		auto foundItr = std::find(pParent->ChildrenIndices.begin(), pParent->ChildrenIndices.end(), [childNode](const NodeIndex& it) { return it == childNode; });
		if (foundItr != pParent->ChildrenIndices.end())
		{
			POLY_CORE_WARN("Cannot add child {} to parent {}, child is already added to parent", childNode, parentNode);
			return;
		}

		pParent->ChildrenIndices.push_back(childNode);
	}

	template<typename T>
	T* HierarchyTree<T>::TraverseNext()
	{
		if (m_TraverseMode == TraverseMode::DEPTH_FIRST || m_TraverseMode == TraverseMode::NONE)
		{
			if (m_CurrentNodeIndex == UINT32_MAX)
				m_CurrentNodeIndex = 0;
			else if (m_TraverseStack.empty())
				return nullptr;
			else
			{
				m_CurrentNodeIndex = m_TraverseStack.top();
				m_TraverseStack.pop();
			}

			for (const auto& index : m_Nodes[m_CurrentNodeIndex].ChildrenIndices)
				m_TraverseStack.push(index);
		}
		else if (m_TraverseMode == TraverseMode::BREADTH_FIRST)
		{
			if (m_CurrentNodeIndex == UINT32_MAX)
				m_CurrentNodeIndex = 0;
			else if (m_TraveseQueue.empty())
				return nullptr;
			else
			{
				m_CurrentNodeIndex = m_TraverseQueue.front();
				m_TraverseQueue.pop();
			}

			for (const auto& index : m_Nodes[m_CurrentNodeIndex].ChildrenIndices)
				m_TraverseQueue.push(index);
		}

		return &(m_Nodes[m_CurrentNodeIndex].Data);
	}

	template<typename T>
	void HierarchyTree<T>::ResetTraverse()
	{
		while (!m_TraverseStack.empty())
			m_TraverseStack.pop();

		while (!m_TraveseQueue.empty())
			m_TraveseQueue.pop();

		m_CurrentNodeIndex = UINT32_MAX;
	}

	template<typename T>
	void HierarchyTree<T>::SetTraverseMode(TraverseMode mode)
	{
		if (mode == TraverseMode::NONE)
		{
			POLY_CORE_WARN("Traverse mode cannot be none!");
			return;
		}

		m_TraverseMode = mode;
	}
}