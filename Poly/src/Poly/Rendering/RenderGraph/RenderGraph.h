#pragma once

#include "Poly/Core/Core.h"

#include <unordered_set>

/**
 * RenderGraph implementation - inspired a lot by the following resources:
 * https://github.com/NVIDIAGameWorks/Falcor/tree/master/Source/Falcor/RenderGraph
 * https://github.com/IbexOmega/CrazyCanvas/tree/master/LambdaEngine/Include/Rendering
 * https://github.com/planetchili/hw3d/tree/master/hw3d
 */

namespace Poly
{
	class RenderPass;
	class Resource;
	class DirectedGraph;

	class RenderGraph
	{
	public:
		// Prefer to use the static Create function to create the render graph
		RenderGraph(const std::string& name);
		~RenderGraph() = default;

		static Ref<RenderGraph> Create(const std::string& name);

		/**
		 * Compiles the rendergraph - all changes to it are final after this point.
		 * If any changes are required a recompilation will also be needed.
		 */
		void Compile();

		/**
		 * Adds a pass to the graph. The name used here is the name that the pass will be set to
		 * and will therefore be the name that is used when interacting with the pass
		 * @param pPass - SharedPtr (Ref) to the RenderPass to be added
		 * @param name - Name of the pass to be added
		 * @return true if pass is added successfully
		 */
		bool AddPass(const Ref<RenderPass>& pPass, const std::string& name);

		/**
		 * Removes a pass from the graph - be aware of loose edges after this has been done
		 * @param name - Name of the pass to be removed
		 * @return true if pass is removed successfully
		 */
		bool Removepass(const std::string& name);

		/**
		 * Add a link to the graph. Depending on the src and destination it will either create a
		 * data-dependency link or a execution-dependency
		 *
		 * - Data-dependency:		By linking two passes' resources to each other a data-dependency will be created.
		 * 							This enforces the graph to keep the data between those passes dependent on each other,
		 * 							removing possible overlap in data usage
		 * 							USAGE: src and dst must follow "renderPassName.resourceName" structure. If resource is external
		 * 									then use the global namespace of $ instead of rennderPassName
		 *
		 * - Execution-dependency:	By linking two passes, without any resource, two each other an execution-dependency will be created.
		 * 							This enforces the graph to keep the execution of the passes in a certain order depending on each other,
		 * 							removing possible overlap in execution
		 * 							USAGE: src and dst must follow "renderPassName" structure
		 *
		 * @param src - src pass/resource name
		 * @param dst - dst pass/resource name
		 * @return true if link is added successfully
		 */
		bool AddLink(const std::string& src, const std::string& dst);

		/**
		 * Remove an already existing link, must follow same structure of names as AddLink
		 * @param src - src pass/resource name
		 * @param dst - dst pass/resource name
		 * @return true if link is removed successfully
		 */
		bool RemoveLink(const std::string& src, const std::string& dst);

		/**
		 * Add a global input resource node to the graph. This resource will be in the global space
		 * which means it will use the $ prefix, i.e. resource name will become $.resource
		 * @param name - Resource name without any prefix - prefix is automatically added
		 * @param pResource - Ref to the resource
		 * @return true if resource could be added successfully
		 */
		bool AddExternalResource(const std::string& name, Ref<Resource> pResource);

		/**
		 * Removes a previously added external resource
		 * @param name - Name of resource to remove
		 * @return true if resource could be removed successfully
		 */
		bool RemoveExternalResource(const std::string& name);

		/**
		 * Marks the given RenderPass name to be an output of the graph
		 * If the graph is the outer most graph then this will be the RenderTarget
		 * @param name	- Resource to mark following renderPass.resource structure
		 * @return true if output could be added successfully
		 */
		bool MarkOutput(const std::string& name);

		/**
		 * Unmarks the given RenderPass name to no longer be an output of the graph
		 * @param name	- Resource to unmark following renderPass.resource structure
		 * @return true if output could be added successfully
		 */
		bool UnmarkOutput(const std::string& name);

	private:
		struct EdgeData
		{
			// If auto generation is added - mark it here
			std::string Src;
			std::string Dst;
		};

		struct Output
		{
			uint32 NodeID;
			std::string ResourceName;

			bool operator== (const Output& other)
			{
				return NodeID == other.NodeID && ResourceName == other.ResourceName;
			}
		};

		/**
		 * @return pair.first = Render pass name, pair.second = resource name belonging to the aforementioned pass
		 */
		std::pair<std::string, std::string> GetPassNameResourcePair(std::string name);

		std::string m_Name = "";
		Ref<DirectedGraph> m_pGraph;
		std::unordered_map<std::string, uint32> m_NameToNodeIndex;
		std::unordered_map<uint32, Ref<RenderPass>> m_Passes;
		std::unordered_map<uint32, EdgeData> m_Edges;
		std::unordered_set<Output> m_Outputs;
		std::unordered_map<std::string, Ref<Resource>> m_ExternalResources;
	};
}