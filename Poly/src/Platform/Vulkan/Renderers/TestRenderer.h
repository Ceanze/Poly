#pragma once

#include "Platform/Common/IRenderer.h"

#include <vector>
#include "Poly/Core/Core.h"

namespace Poly
{

	class VulkanRenderer;
	class SwapChain;
	class GraphicsPipeline;
	class PipelineLayout;
	class Shader;
	class RenderPass;
	class Framebuffer;
	class CommandPool;
	class CommandBuffer;
	class DescriptorSet;
	class Buffer;
	class Texture;
	class TextureView;
	class Sampler;

	class TestRenderer : public IRenderer
	{
	public:
		TestRenderer() = default;
		virtual ~TestRenderer() override;
		virtual void SetWindow(Window* window) override { m_pWindow = window; }
		virtual void Init(IPlatformRenderer* pRenderer) override;
		virtual void BeginScene(uint32_t imageIndex) override;
		virtual void Record() override;
		virtual void EndScene() override;

	private:
		void CreateCommandBuffers();
		void SetupDescriptorSet();
		void SetupRenderPass();
		void SetupPipelineLayout();
		void SetupPipeline();
		void SetupTestData();

		VulkanRenderer*	m_pMainRenderer	= nullptr;
		SwapChain*		m_pSwapChain	= nullptr;
		uint32_t		m_ImageIndex	= 0;

		Ref<GraphicsPipeline>	m_Pipeline;
		Ref<Shader>				m_VertShader;
		Ref<Shader>				m_FragShader;
		Ref<RenderPass>			m_RenderPass;
		std::vector<Ref<Framebuffer>> m_Framebuffers;
		Ref<CommandPool>		m_CommandPool;
		std::vector<CommandBuffer*> m_CommandBuffers;
		std::vector<Ref<DescriptorSet>>	m_DescriptorSets;
		Ref<PipelineLayout>		m_PipelineLayout;

		Ref<Buffer>			m_TestBuffer;
		Ref<TextureView>	m_TestTextureView;
		Ref<Texture>		m_TestTexture;
		Sampler*			m_pTestSampler = nullptr;

		Window*			m_pWindow = nullptr;
	};

}