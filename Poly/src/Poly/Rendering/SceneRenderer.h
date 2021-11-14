#pragma once
#include "Scene.h"

namespace Poly
{
	struct SceneBinding;

	class Scene;
	class CommandBuffer;
	class DescriptorSet;
	class PipelineLayout;

	enum class ResourceKey
	{
		NONE		= 0,
		VERTEX		= 1,
		TEXTURE		= 2,
		INSTANCE	= 3,
	};

	struct FramePassKey
	{
		uint32		FrameIndex;
		uint32		ImageIndex;
		ResourceKey Key;

		bool operator==(const FramePassKey& other) const
		{
			return other.FrameIndex == FrameIndex && other.ImageIndex == ImageIndex && other.Key == Key;
		}
	};

	struct FramePassHasher
	{
		size_t operator()(const FramePassKey& other) const
		{
			return (static_cast<uint64>(other.FrameIndex) << 48) | (static_cast<uint64>(other.ImageIndex) << 32) | static_cast<uint64>(other.Key);
		}
	};

	class SceneRenderer
	{
	public:
		SceneRenderer() = default;
		~SceneRenderer() = default;

		void SetScene(Ref<Scene> pScene) { m_pScene = pScene; }

		void Update(const std::vector<SceneBinding>& sceneBindings, uint32 imageIndex, uint32 passIndex, PipelineLayout* pPipelineLayout);

		void Render(const RenderContext& context);

		static Ref<SceneRenderer> Create() { return CreateRef<SceneRenderer>(); }

	private:
		// using FramePassKey = std::pair<uint32, uint32>;

		Ref<DescriptorSet> GetDescriptor(FramePassKey framePassKey, uint32 drawObjectIndex, uint32 setIndex, PipelineLayout* pPipelineLayout);

		Ref<Scene> m_pScene;

		std::unordered_map<FramePassKey, std::vector<Ref<DescriptorSet>>, FramePassHasher> m_pVertexDescriptors;
		std::unordered_map<size_t, DrawObject> m_DrawObjects;
	};

}