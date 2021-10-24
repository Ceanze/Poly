#pragma once
#include "Scene.h"

namespace Poly
{
	class Scene;
	class CommandBuffer;
	class DescriptorSet;
	class PipelineLayout;

	class SceneRenderer
	{
	public:
		SceneRenderer() = default;
		~SceneRenderer() = default;

		void SetScene(Ref<Scene> pScene) { m_pScene = pScene; }

		void Update(uint32 setIndex, uint32 imageIndex, uint32 passIndex, PipelineLayout* pPipelineLayout);

		void Render(const RenderContext& context);

		static Ref<SceneRenderer> Create() { return CreateRef<SceneRenderer>(); }

	private:
		using FramePassKey = std::pair<uint32, uint32>;

		Ref<DescriptorSet> GetDescriptor(FramePassKey framePassKey, uint32 drawObjectIndex, uint32 setIndex, PipelineLayout* pPipelineLayout);

		Ref<Scene> m_pScene;

		std::unordered_map<FramePassKey, std::vector<Ref<DescriptorSet>>, FramePassHasher> m_pVertexDescriptors;
		std::unordered_map<size_t, DrawObject> m_DrawObjects;
	};

}