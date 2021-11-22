#pragma once
#include "Scene.h"

namespace Poly
{
	struct SceneBinding;

	class Scene;
	class Buffer;
	class RenderContext;
	class CommandBuffer;
	class DescriptorSet;
	class PipelineLayout;

	struct FramePassKey
	{
		uint32	FrameIndex;
		uint32	ImageIndex;
		uint32	Set;

		bool operator==(const FramePassKey& other) const
		{
			return other.FrameIndex == FrameIndex && other.ImageIndex == ImageIndex && other.Set == Set;
		}
	};

	struct FramePassHasher
	{
		size_t operator()(const FramePassKey& other) const
		{
			return (static_cast<uint64>(other.FrameIndex) << 48) | (static_cast<uint64>(other.ImageIndex) << 32) | static_cast<uint64>(other.Set);
		}
	};

	class SceneRenderer
	{
	public:
		SceneRenderer() = default;
		~SceneRenderer() = default;

		void SetScene(Ref<Scene> pScene) { m_pScene = pScene; }

		void Update(const RenderContext& context, const std::vector<SceneBinding>& sceneBindings, uint32 imageIndex, PipelineLayout* pPipelineLayout);

		void Render(const RenderContext& context);

		static Ref<SceneRenderer> Create() { return CreateRef<SceneRenderer>(); }

	private:
		Ref<DescriptorSet> GetDescriptor(FramePassKey framePassKey, uint32 drawObjectIndex, uint32 setIndex, PipelineLayout* pPipelineLayout);
		void UpdateBuffers(uint64 size);

		Ref<Scene> m_pScene;
		uint32 m_MaxInstanceSize = 0;

		Ref<Buffer> m_pInstanceBuffer;
		Ref<Buffer> m_pStagingBuffer;

		std::unordered_map<FramePassKey, std::vector<Ref<DescriptorSet>>, FramePassHasher> m_Descriptors;
		std::unordered_map<size_t, DrawObject> m_DrawObjects;
	};

}