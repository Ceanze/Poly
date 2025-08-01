#pragma once

#include "RenderGraphTypes.h"
#include "PassReflection.h"
#include "ResourceView.h"
#include "ResourceGUID.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Poly/Rendering/Utilities/DescriptorCache.h"

#include <set>

namespace Poly
{
	class Pass;
	class Scene;
	class Resource;
	class Framebuffer;
	class RenderGraph;
	class CommandPool;
	class DescriptorSet;
	class CommandBuffer;
	class ResourceCache;
	class PipelineLayout;
	class GraphicsPipeline;
	class GraphicsRenderPass;
	class StagingBufferCache;


	class RenderGraphProgram
	{
	public:
		RenderGraphProgram(RenderGraph* pRenderGraph, Ref<ResourceCache> pResourceCache, RenderGraphDefaultParams defaultParams);
		~RenderGraphProgram() = default;

		/**
		 * USED BY THE RENDER GRAPH
		 */
		void Init();

		/**
		 * USED BY THE RENDER GRAPH COMPILER
		 */
		static Ref<RenderGraphProgram> Create(RenderGraph* pRenderGraph, Ref<ResourceCache> pResourceCache, RenderGraphDefaultParams defaultParams);

		/**
		 * USED BY THE RENDERER
		 */
		void Execute(uint32 imageIndex);

		/**
		* Creates a new resource, if not already existing, for the given resource GUID, and tranfers the optionally provided data
		* @param resourceGUID - name of resource to create. Note: Resource must be in the global/external space, i.e. the prefix is empty or $.
		*						Internal resources between passes are created automatically when an output from a pass exist, see RenderGraph for more
		 * @param size - size of the data to update with
		 * @param data - pointer to the data, nullptr if no data
		 * @param bufferUsage - the usage of the buffer, usually STORAGE_BUFFER or UNIFORM_BUFFER. TRANSFER_DST is inferred
		 * @param offset - offset of the descriptor to update - remember to check the offset of the data as well
		 * @param index - index of descriptor to update if multiple resources per binding point
		*/
		void CreateResource(ResourceGUID resourceGUID, uint64 size, const void* data, FBufferUsage bufferUsage, uint64 offset = 0, uint32 index = 0);

		/**
		* @return true if resource exists, false otherwise
		*/
		bool HasResource(ResourceGUID resourceGUID) const;

		/**
		 * Updates an existing resource's descriptor - must be done when the resource has changed size or if
		 * the name's resouces is a new resource
		 * @param name - name of resource follow "renderPass.resource" or "$.resource" format, no prefix assumes external
		 * @param pResource - resource to update with - nullptr if resource is same and only updated binding. If resource is provided, all bindings using this resource will also be updated.
		 * @param index - index of descriptor to update if multiple resources per binding point
		 */
		void UpdateGraphResource(ResourceGUID resourceGUID, const Resource* pResource, uint32 index = 0);

		/**
		 * Updates an existing resource's descriptor - must be done when the resource has changed size or if
		 * the name's resources is a new resource
		 * @param name - name of resource follow "renderPass.resource" or "$.resource" format, no prefix assumes external
		 * @param view - ResourceView of the resource/buffer/textureView to update, ResourceView::Empty or empty inner resource if resource is same and only updated binding. If resource is provided, all bindings using this resource will also be updated.
		 * @param offset - offset of the descriptor to update - remember to check the offset of the ResourceView as well
		 * @param index - index of descriptor to update if multiple resources per binding point
		 */
		void UpdateGraphResource(ResourceGUID resourceGUID, ResourceView view, uint32 index = 0);

		/**
		 * Updates an existing resource's descriptor - must be done when the resource has changed size or if
		 * the name's resouces is a new resource
		 * @param name - name of resource follow "renderPass.resource" or "$.resource" format, no prefix assumes external
		 * @param size - size of the data to update with. If size differs than already existing buffer all bindings using this resource will also be updated.
		 * @param data - pointer to the new data
		 * @param offset - offset of the descriptor to update - remember to check the offset of the data as well
		 * @param index - index of descriptor to update if multiple resources per binding point
		 */
		void UpdateGraphResource(ResourceGUID resourceGUID, uint64 size, const void* data, uint64 offset = 0, uint32 index = 0);

		/**
		 * USED BY THE RENDERER
		 */
		void SetBackbuffer(Ref<Resource> pResource);

		/**
		 * Sets the active scene that can be accessed with the render data in render graph passes to be rendered
		 * @param pScene - scene to target
		 */
		void SetScene(const Ref<Scene>& pScene);

		/**
		 * Gets the active scene
		 * @return active scene - nullptr if no active scene
		 */
		const Scene* GetScene() const { return m_pScene.get(); }

	private:
		friend class RenderGraphCompiler;

		void AddPass(Ref<Pass> pPass); // Used by render graph compiler
		void InitCommandBuffers();
		void InitPipelineLayouts();
		GraphicsRenderPass* GetGraphicsRenderPass(const Ref<Pass>& pPass, uint32 passIndex);
		Framebuffer* GetFramebuffer(const Ref<Pass>& pPass, uint32 passIndex);
		GraphicsPipeline* GetGraphicsPipeline(const Ref<Pass>& pPass, uint32 passIndex);

		ResourceGUID GetMappedResourceGUID(const ResourceGUID& resourceGUID, const Ref<Pass>& pPass, uint32 passIndex);

		// General
		Ref<Scene> m_pScene;
		Ref<StagingBufferCache> m_pStagingBufferCache;

		// Render Graph specific types
		std::vector<Ref<Pass>>		m_Passes;
		Ref<ResourceCache>			m_pResourceCache;
		RenderGraphDefaultParams	m_DefaultParams;
		RenderGraph*				m_pRenderGraph;
		std::unordered_map<uint32, PassReflection>	m_Reflections;

		// Rendering specific types
		uint32														m_ImageIndex = 0;
		std::vector<std::vector<CommandBuffer*>>					m_CommandBuffers; // buffs[passIndex][imageIndex]
		std::unordered_map<FQueueType, Ref<CommandPool>>			m_CommandPools;
		std::unordered_map<uint32, Ref<GraphicsRenderPass>>			m_GraphicsRenderPasses; // key: passIndex
		std::unordered_map<uint32, std::vector<Ref<Framebuffer>>>	m_Framebuffers; // key: passIndex, index: imageIndex
		std::unordered_map<uint32, Ref<PipelineLayout>>				m_PipelineLayouts; // key: passIndex
		std::unordered_map<uint32, Ref<GraphicsPipeline>>			m_GraphicsPipelines; // key: passIndex
		std::unordered_map<uint32, DescriptorCache>					m_DescriptorCaches;
	};
}