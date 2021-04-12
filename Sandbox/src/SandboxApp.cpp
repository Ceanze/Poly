#include "Poly.h"
#include "Poly/Rendering/Renderer.h"
#include "Poly/Rendering/RenderGraph/RenderGraphCompiler.h"
#include "Poly/Rendering/RenderGraph/RenderGraphProgram.h"
#include "Poly/Rendering/RenderGraph/RenderGraph.h"
#include "Poly/Rendering/RenderGraph/Passes/TestPass.h"
#include "Poly/Core/RenderAPI.h"
#include "Poly/Rendering/RenderGraph/Resource.h"
#include "Platform/API/TextureView.h"
#include "Platform/API/Texture.h"
#include "Platform/API/Buffer.h"
#include "Poly/Resources/ResourceLoader.h"

class TestLayer : public Poly::Layer
{
public:
	TestLayer() 
	{
		// Poly::RendererAPI::Create(Poly::BackendAPI::VULKAN);
		// Poly::RendererAPI::CreateRenderer(Poly::Renderer::TEST);
		 pCamera = new Poly::Camera();
		 pCamera->SetAspect(1280.f / 720.f);
		 pCamera->SetMouseSense(3.f);
		
		// Poly::RendererAPI::SetActiveCamera(pCamera);

		// Poly::RendererAPI::Init(1280, 720);

		// Creation
		m_pRenderer = Poly::Renderer::Create();
		m_pGraph = Poly::RenderGraph::Create("TestGraph");
		Poly::Ref<Poly::Pass> pPass = Poly::TestPass::Create();

		// External resources
		Poly::Ref<Poly::Texture> pTexture = Poly::ResourceLoader::LoadTexture("textures/ceanze.png", Poly::EFormat::R8G8B8A8_UNORM);
		Poly::TextureViewDesc textureViewDesc = {
			.pTexture			= pTexture.get(),
			.ImageViewType		= Poly::EImageViewType::TYPE_2D,
			.Format				= Poly::EFormat::R8G8B8A8_UNORM,
			.ImageViewFlag		= Poly::FImageViewFlag::COLOR,
			.MipLevel			= 0,
			.MipLevelCount		= 1,
			.ArrayLayer			= 0,
			.ArrayLayerCount	= 1,
		};
		Poly::Ref<Poly::TextureView> pTextureView = Poly::RenderAPI::CreateTextureView(&textureViewDesc);

		Poly::BufferDesc bufferDesc = {
			.Size			= sizeof(glm::mat4),
			.MemUsage		= Poly::EMemoryUsage::CPU_GPU_MAPPABLE,
			.BufferUsage	= Poly::FBufferUsage::UNIFORM_BUFFER
		};
		m_pCambuffer = Poly::RenderAPI::CreateBuffer(&bufferDesc);

		Poly::Ref<Poly::Resource> pCamRes = Poly::Resource::Create(m_pCambuffer, "camera");
		Poly::Ref<Poly::Resource> pTexRes = Poly::Resource::Create(pTexture, pTextureView, "texture");
		m_pGraph->AddExternalResource("camera", pCamRes);
		m_pGraph->AddExternalResource("texture", pTexRes);

		// Passes and links
		m_pGraph->AddPass(pPass, "testPass");
		m_pGraph->AddLink("$.camera", "testPass.camera");
		m_pGraph->AddLink("$.texture", "testPass.texture");
		m_pGraph->MarkOutput("testPass.out");

		// Compile
		m_pProgram = m_pGraph->Compile();

		// Update resources to validate descriptors
		// TODO: Do this automatically in the beginning of the program
		m_pProgram->UpdateExternalResource("camera", pCamRes);
		m_pProgram->UpdateExternalResource("texture", pTexRes);

		// Set active render graph program
		m_pRenderer->SetRenderGraph(m_pProgram);
	};

	void OnUpdate(Poly::Timestamp dt) override
	{
		//POLY_INFO("Testlayer update!");
		pCamera->Update(dt);
		glm::mat4 camMatrix = pCamera->GetMatrix();
		m_pCambuffer->TransferData(&camMatrix, sizeof(glm::mat4));
		// Poly::RendererAPI::BeginScene();
		// Poly::RendererAPI::EndScene();
		m_pRenderer->Render();
	};

	void OnDetach() override
	{
		delete pCamera;
	}

private:
	Poly::Camera* pCamera = nullptr;
	Poly::Ref<Poly::Buffer> m_pCambuffer = nullptr;
	Poly::Ref<Poly::Renderer> m_pRenderer = nullptr;
	Poly::Ref<Poly::RenderGraph> m_pGraph = nullptr;
	Poly::Ref<Poly::RenderGraphProgram> m_pProgram = nullptr;
};

class Sandbox : public Poly::Application
{
public:
	Sandbox()
	{
		PushLayer(new TestLayer());
	}

	~Sandbox()
	{
		// Poly::RendererAPI::Shutdown();
	}
};

Poly::Application* Poly::CreateApplication()
{
	return new Sandbox();
}