#include "Poly.h"
#include "Poly/Rendering/Scene.h"
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
#include "Poly/Resources/ResourceManager.h"

class TestLayer : public Poly::Layer
{
public:
	TestLayer() 
	{
		pCamera = new Poly::Camera();
		pCamera->SetAspect(1280.f / 720.f);
		pCamera->SetMouseSense(3.f);
		

		// Creation
		m_pRenderer = Poly::Renderer::Create();
		m_pGraph = Poly::RenderGraph::Create("TestGraph");
		Poly::Ref<Poly::Pass> pPass = Poly::TestPass::Create();
		Poly::Ref<Poly::Pass> pPass1 = Poly::TestPass::Create();

		// External resources
		PolyID textureID = Poly::ResourceManager::LoadTexture("textures/ceanze.png", Poly::EFormat::R8G8B8A8_UNORM);

		Poly::BufferDesc bufferDesc = {
			.Size			= sizeof(glm::mat4),
			.MemUsage		= Poly::EMemoryUsage::CPU_GPU_MAPPABLE,
			.BufferUsage	= Poly::FBufferUsage::UNIFORM_BUFFER
		};
		m_pCambuffer = Poly::RenderAPI::CreateBuffer(&bufferDesc);

		Poly::ManagedTexture managedTexture = Poly::ResourceManager::GetManagedTexture(textureID);
		Poly::Ref<Poly::Resource> pCamRes = Poly::Resource::Create(m_pCambuffer, "camera");
		Poly::Ref<Poly::Resource> pTexRes = Poly::Resource::Create(managedTexture.pTexture, managedTexture.pTextureView, "texture");
		m_pGraph->AddExternalResource("camera", pCamRes);
		m_pGraph->AddExternalResource("texture", pTexRes);

		// Passes and links
		m_pGraph->AddPass(pPass, "testPass");
		// m_pGraph->AddPass(pPass1, "testPass1");
		m_pGraph->AddLink("$.camera", "testPass.camera");
		m_pGraph->AddLink("$.texture", "testPass.texture");
		// m_pGraph->AddLink("$.camera", "testPass1.camera");
		// m_pGraph->AddLink("testPass.out", "testPass1.texture");
		m_pGraph->MarkOutput("testPass.out");

		// Compile
		m_pProgram = m_pGraph->Compile();

		// Update resources to validate descriptors
		// TODO: Do this automatically in the beginning of the program
		m_pProgram->UpdateGraphResource("camera", pCamRes);
		m_pProgram->UpdateGraphResource("texture", pTexRes);

		Poly::Ref<Poly::Scene> pScene = Poly::Scene::Create();
		m_pProgram->SetScene(pScene);

		PolyID modelIDCube = Poly::ResourceManager::LoadModel("../assets/models/cube2.obj");
		pScene->AddModel(modelIDCube);

		// Set active render graph program
		m_pRenderer->SetRenderGraph(m_pProgram);
	};

	void OnUpdate(Poly::Timestamp dt) override
	{
		//POLY_INFO("Testlayer update!");
		pCamera->Update(dt);
		glm::mat4 camMatrix = pCamera->GetMatrix();
		m_pCambuffer->TransferData(&camMatrix, sizeof(glm::mat4));
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