#pragma once

#include "RenderGraphTypes.h"

namespace Poly
{
	class Resource;
	class SceneRenderer;
	class ResourceCache;

	/**
	 * Render Data is created for each execute call of a pass.
	 * The class contains the data that might be needed to execute,
	 * especially the resources with the same name as the reflection.
	 */
	class RenderData
	{
	public:
		RenderData(Ref<ResourceCache> pResourceCache, RenderGraphDefaultParams defaultParams);
		~RenderData() = default;

		/**
		 * Get a resource with the same name given in Reflect
		 * @return Resource
		 */
		const Resource* operator[] (const std::string& resourceName) const;

		const RenderGraphDefaultParams& GetDefaultParams() const { return m_DefaultParams; }

		const SceneRenderer* GetSceneRenderer() const { return m_pSceneRenderer; }

		void ExecuteScene(const RenderContext& context) const;

	private:
		friend class SyncPass;
		friend class RenderGraphProgram;
		Resource* GetResourceNonConst(const std::string& resourceName) const;
		void SetRenderPassName(const std::string& renderPassName) { m_RenderPassName = renderPassName; }
		void SetSceneRenderer(SceneRenderer* pSceneRenderer) { m_pSceneRenderer = pSceneRenderer; }

		Ref<ResourceCache> m_pResourceCache;
		std::string m_RenderPassName = "";
		RenderGraphDefaultParams m_DefaultParams = {};
		SceneRenderer* m_pSceneRenderer = nullptr;
	};
}