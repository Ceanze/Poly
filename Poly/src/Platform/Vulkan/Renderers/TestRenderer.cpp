#include "polypch.h"
#include "TestRenderer.h"

#include "Poly/Core/Camera.h"
#include "Platform/Vulkan/VulkanRenderer.h"

#include "Poly/Core/RenderAPI.h"
#include "Poly/Resources/ResourceLoader.h"

#include "Platform/API/Shader.h"
#include "Platform/API/GraphicsPipeline.h"
#include "Platform/API/GraphicsRenderPass.h"
#include "Platform/API/DescriptorSet.h"
#include "Platform/API/PipelineLayout.h"
#include "Platform/API/CommandPool.h"
#include "Platform/API/CommandBuffer.h"
#include "Platform/API/Framebuffer.h"
#include "Platform/API/TextureView.h"
#include "Platform/API/Texture.h"
#include "Platform/API/CommandQueue.h"
#include "Platform/API/Buffer.h"
#include "Platform/API/Sampler.h"

// TEMP - TESTING OF RENDER GRAPH
#include "Poly/Rendering/RenderGraph/RenderGraph.h"
#include "Poly/Rendering/RenderGraph/Passes/TestPass.h"
#include "Poly/Rendering/RenderGraph/Passes/TestPass1.h"

namespace Poly
{

	TestRenderer::~TestRenderer()
	{
		RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->Wait();
	}

	void TestRenderer::Init(IPlatformRenderer* renderer)
	{
		m_pMainRenderer = static_cast<VulkanRenderer*>(renderer);
		m_pSwapChain = m_pMainRenderer->GetSwapChain();

		m_VertShader = ResourceLoader::LoadShader("vert.glsl", FShaderStage::VERTEX);
		m_FragShader = ResourceLoader::LoadShader("frag.glsl", FShaderStage::FRAGMENT);

		SetupRenderPass();
		SetupPipelineLayout();
		SetupPipeline();
		SetupDescriptorSet();

		m_CommandPool = RenderAPI::CreateCommandPool(FQueueType::GRAPHICS);

		m_Framebuffers.resize(m_pSwapChain->GetBackbufferCount());
		for (uint32 i = 0; i < m_pSwapChain->GetBackbufferCount(); i++)
		{
			FramebufferDesc desc = {};
			desc.Height			= m_pSwapChain->GetDesc().Height;
			desc.Width			= m_pSwapChain->GetDesc().Width;
			desc.pRenderPass	= m_RenderPass.get();
			desc.pTextureView	= m_pSwapChain->GetTextureView(i);

			m_Framebuffers[i] = RenderAPI::CreateFramebuffer(&desc);
		}

		SetupTestData();
		CreateCommandBuffers();

		// TEMP - TESTING OF RENDER GRAPH
		Ref<RenderGraph> rg = RenderGraph::Create("Test graph");
		auto pass0 = TestPass::Create();
		auto pass1 = TestPass1::Create();

		rg->AddPass(pass0, "0");
		rg->AddPass(pass1, "1");

		rg->AddLink("0.a", "1.a");

		rg->MarkOutput("1.b");

		rg->Compile();
	}

	void TestRenderer::BeginScene(uint32_t imageIndex)
	{
		m_ImageIndex = imageIndex;
	}

	void TestRenderer::Record()
	{
	}

	void TestRenderer::EndScene()
	{
		m_pMainRenderer->AddCommandBuffer(FQueueType::GRAPHICS, m_CommandBuffers[m_ImageIndex]);
		glm::mat4 camMatrix = m_pCamera->GetMatrix();
		m_TestBuffer->TransferData(&camMatrix, sizeof(glm::mat4));
	}

	void TestRenderer::CreateCommandBuffers()
	{
		m_CommandBuffers.resize(m_pSwapChain->GetBackbufferCount());
		for (uint32_t i = 0; i < m_CommandBuffers.size(); i++) {
			m_CommandBuffers[i] = m_CommandPool->AllocateCommandBuffer(ECommandBufferLevel::PRIMARY);

			float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

			m_CommandBuffers[i]->Begin(FCommandBufferFlag::NONE);
			m_CommandBuffers[i]->BeginRenderPass(m_RenderPass.get(), m_Framebuffers[i].get(), m_pSwapChain->GetDesc().Width, m_pSwapChain->GetDesc().Height, clearColor, 1);
			m_CommandBuffers[i]->BindPipeline(m_Pipeline.get());
			m_CommandBuffers[i]->BindDescriptor(m_Pipeline.get(), m_DescriptorSets[i].get());
			m_CommandBuffers[i]->DrawInstanced(3, 1, 0, 0);
			m_CommandBuffers[i]->EndRenderPass();
			m_CommandBuffers[i]->End();
		}
	}

	void TestRenderer::SetupDescriptorSet()
	{
		m_DescriptorSets.resize(m_pSwapChain->GetBackbufferCount());
		for (uint32 i = 0; i < m_DescriptorSets.size(); i++)
			m_DescriptorSets[i] = RenderAPI::CreateDescriptorSet(m_PipelineLayout.get(), 0);
	}

	void TestRenderer::SetupRenderPass()
	{
		GraphicsRenderPassAttachmentDesc attachmentDesc = {};
		attachmentDesc.Format			= m_pSwapChain->GetDesc().Format;
		attachmentDesc.SampleCount		= 1;
		attachmentDesc.LoadOp			= ELoadOp::CLEAR;
		attachmentDesc.StoreOp			= EStoreOp::STORE;
		attachmentDesc.StencilLoadOp	= ELoadOp::DONT_CARE;
		attachmentDesc.StencilStoreOp	= EStoreOp::DONT_CARE;
		attachmentDesc.InitialLayout	= ETextureLayout::UNDEFINED;
		attachmentDesc.FinalLayout		= ETextureLayout::PRESENT;

		GraphicsRenderPassSubpassAttachmentReference attachmentRef = {};
		attachmentRef.Index		= 0;
		attachmentRef.Layout	= ETextureLayout::COLOR_ATTACHMENT_OPTIMAL;

		GraphicsRenderPassSubpassDesc subpassDesc = {};
		subpassDesc.ColorAttachmentsLayouts = { attachmentRef };

		GraphicsRenderPassSubpassDependencyDesc depDesc = {};
		depDesc.SrcSubpass		= EXTERNAL_SUBPASS;
		depDesc.DstSubpass		= 0;
		depDesc.SrcStageMask	= FPipelineStage::COLOR_ATTACHMENT_OUTPUT;
		depDesc.SrcAccessMask	= FAccessFlag::NONE;
		depDesc.DstStageMask	= FPipelineStage::COLOR_ATTACHMENT_OUTPUT;
		depDesc.DstAccessMask	= FAccessFlag::COLOR_ATTACHMENT_READ | FAccessFlag::COLOR_ATTACHMENT_WRITE;

		GraphicsRenderPassDesc renderPassDesc = {};
		renderPassDesc.Attachments			= { attachmentDesc };
		renderPassDesc.Subpasses			= { subpassDesc };
		renderPassDesc.SubpassDependencies	= { depDesc };

		m_RenderPass = RenderAPI::CreateRenderPass(&renderPassDesc);
	}

	void TestRenderer::SetupPipelineLayout()
	{
		DescriptorSetBinding binding0 = {};
		binding0.Binding			= 0;
		binding0.DescriptorCount	= 1;
		binding0.DescriptorType		= EDescriptorType::UNIFORM_BUFFER;
		binding0.ShaderStage		= FShaderStage::VERTEX;

		DescriptorSetBinding binding1 = {};
		binding1.Binding			= 1;
		binding1.DescriptorCount	= 1;
		binding1.DescriptorType		= EDescriptorType::COMBINED_IMAGE_SAMPLER;
		binding1.ShaderStage		= FShaderStage::FRAGMENT;

		DescriptorSetLayout layout = {};
		layout.DescriptorSetBindings = { binding0, binding1 };

		PipelineLayoutDesc desc = {};
		desc.DescriptorSetLayouts = { layout };

		m_PipelineLayout = RenderAPI::CreatePipelineLayout(&desc);
	}

	void TestRenderer::SetupPipeline()
	{
		InputAssemblyDesc assembly = {};
		assembly.RestartPrimitive	= false;
		assembly.Topology			= ETopology::TRIANGLE_LIST;

		ViewportDesc viewport = {};
		viewport.Width		= static_cast<float>(m_pSwapChain->GetDesc().Width);
		viewport.Height		= static_cast<float>(m_pSwapChain->GetDesc().Height);

		ScissorDesc scissor = {};
		scissor.Width		= m_pSwapChain->GetDesc().Width;
		scissor.Height		= m_pSwapChain->GetDesc().Height;

		RasterizationDesc raster = {};
		raster.DepthClampEnable		= false;
		raster.DiscardEnable		= false;
		raster.PolygonMode			= EPolygonMode::FILL;
		raster.CullMode				= ECullMode::NONE;
		raster.ClockwiseFrontFace	= false;
		raster.DepthBiasEnable		= false;

		ColorBlendAttachmentDesc colorBlendAttachment = {};
		colorBlendAttachment.ColorWriteMask		= FColorComponentFlag::RED | FColorComponentFlag::GREEN | FColorComponentFlag::BLUE | FColorComponentFlag::ALPHA;
		colorBlendAttachment.BlendEnable		= false;

		ColorBlendStateDesc colorBlend = {};
		colorBlend.LogicOpEnable			= false;
		colorBlend.ColorBlendAttachments	= { colorBlendAttachment };

		GraphicsPipelineDesc desc = {};
		desc.InputAssembly		= assembly;
		desc.Viewport			= viewport;
		desc.Scissor			= scissor;
		desc.Rasterization		= raster;
		desc.ColorBlendState	= colorBlend;
		desc.pPipelineLayout	= m_PipelineLayout.get();
		desc.pRenderPass		= m_RenderPass.get();
		desc.pVertexShader		= m_VertShader.get();
		desc.pFragmentShader	= m_FragShader.get();

		m_Pipeline = RenderAPI::CreateGraphicsPipeline(&desc);
	}

	void TestRenderer::SetupTestData()
	{
		m_pTestSampler = Sampler::GetDefaultLinearSampler();

		 //TextureDesc textureDesc = {
		 //	.Width			= 1,
		 //	.Height			= 1,
		 //	.Depth			= 1,
		 //	.ArrayLayers	= 1,
		 //	.MipLevels		= 1,
		 //	.SampleCount	= 1,
		 //	.MemoryUsage	= EMemoryUsage::GPU_ONLY,
		 //	.Format			= EFormat::R8G8B8A8_UNORM,
		 //	.TextureUsage	= FTextureUsage::SAMPLED,
		 //	.TextureDim		= ETextureDim::DIM_2D,
		 //};
		 //m_TestTexture = RenderAPI::CreateTexture(&textureDesc);

		m_TestTexture = ResourceLoader::LoadTexture("textures/ceanze.png", EFormat::R8G8B8A8_UNORM);

		TextureViewDesc textureViewDesc = {
			.pTexture			= m_TestTexture.get(),
			.ImageViewType		= EImageViewType::TYPE_2D,
			.Format				= EFormat::R8G8B8A8_UNORM,
			.ImageViewFlag		= FImageViewFlag::COLOR,
			.MipLevel			= 0,
			.MipLevelCount		= 1,
			.ArrayLayer			= 0,
			.ArrayLayerCount	= 1,
		};
		m_TestTextureView = RenderAPI::CreateTextureView(&textureViewDesc);

		BufferDesc bufferDesc = {
			.Size			= sizeof(glm::mat4),
			.MemUsage		= EMemoryUsage::CPU_GPU_MAPPABLE,
			.BufferUsage	= FBufferUsage::UNIFORM_BUFFER
		};
		m_TestBuffer = RenderAPI::CreateBuffer(&bufferDesc);

		for (uint32 i = 0; i < m_DescriptorSets.size(); i++)
		{
			m_DescriptorSets[i]->UpdateBufferBinding(0, m_TestBuffer.get(), 0, m_TestBuffer->GetSize());
			m_DescriptorSets[i]->UpdateTextureBinding(1, ETextureLayout::SHADER_READ_ONLY_OPTIMAL, m_TestTextureView.get(), m_pTestSampler);
		}
	}

}