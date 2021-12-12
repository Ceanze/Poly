#pragma once

#include "../RenderPass.h"

namespace Poly
{
	class Buffer;
	class Sampler;
	class Texture;
	class TextureView;
	class StagingBufferCache;

	class ImGuiPass : public RenderPass
	{
	private:
		struct PushConstantBlock
		{
			glm::vec2 scale;
			glm::vec2 translate;
		};

	public:
		ImGuiPass();
		~ImGuiPass();

		virtual PassReflection Reflect() override final;

		virtual void Compile() override final;

		virtual void Update(const RenderContext& context) override final;

		virtual void Execute(const RenderContext& context, const RenderData& renderData) override final;

		static Ref<ImGuiPass> Create() { return CreateRef<ImGuiPass>(); }

	private:
		void SetupCustomPipeline();
		void SetupFont();
		void UpdateBuffers(uint32 imageIndex);

		PushConstantBlock m_PushConstantData;

		Ref<Texture>		m_pFontTexture;
		Ref<TextureView>	m_pFontTextureView;
		Ref<Sampler>		m_pFontSampler;
		Ref<Buffer>			m_pVertexBuffer;
		Ref<Buffer>			m_pIndexBuffer;

		Ref<StagingBufferCache> m_pStagingBufferCache;

		std::unordered_map<uint32, std::vector<Ref<Buffer>>> m_BuffersToBeDestroyed;
	};
}