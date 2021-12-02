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
		pCamera->SetMouseSense(2.f);
		pCamera->SetMovementSpeed(3.f);
		pCamera->SetSprintSpeed(6.f);

		// Creation
		m_pRenderer = Poly::Renderer::Create();
		m_pGraph = Poly::RenderGraph::Create("TestGraph");
		Poly::Ref<Poly::Pass> pPass = Poly::TestPass::Create();

		// External resources
		// Poly::BufferDesc bufferDesc = {
		// 	.Size			= sizeof(glm::mat4),
		// 	.MemUsage		= Poly::EMemoryUsage::CPU_GPU_MAPPABLE,
		// 	.BufferUsage	= Poly::FBufferUsage::UNIFORM_BUFFER
		// };
		// m_pCambuffer = Poly::RenderAPI::CreateBuffer(&bufferDesc);

		// Poly::Ref<Poly::Resource> pCamRes = Poly::Resource::Create(m_pCambuffer, "camera");
		// m_pGraph->AddExternalResource("camera", pCamRes);
		m_pGraph->AddExternalResource("camera", sizeof(glm::mat4), Poly::FBufferUsage::UNIFORM_BUFFER);

		// Passes and links
		m_pGraph->AddPass(pPass, "testPass");
		m_pGraph->AddLink("$.camera", "testPass.camera");
		m_pGraph->MarkOutput("testPass.out");

		// Compile
		m_pProgram = m_pGraph->Compile();

		// Update resources to validate descriptors
		// TODO: Do this automatically in the beginning of the program
		m_pProgram->UpdateGraphResource("camera", nullptr);

		Poly::Ref<Poly::Scene> pScene = Poly::Scene::Create();
		m_pProgram->SetScene(pScene);

		// PolyID sponza = Poly::ResourceManager::LoadModel("../assets/models/sponza/glTF/Sponza.gltf");
		PolyID sponza = Poly::ResourceManager::LoadModel("../assets/models/sponza/glTF-Binary/Sponza.glb");
		PolyID cube = Poly::ResourceManager::LoadModel("../assets/models/Cube/Cube.gltf");
		// PolyID cube1 = Poly::ResourceManager::LoadModel("../assets/models/Cube/Cube.gltf");
		pScene->AddModel(sponza);
		pScene->AddModel(cube);
		// pScene->AddModel(cube1);

		// Scale the giant sponza
		// Poly::ResourceManager::GetModel(sponza)->SetScale(0.05f);

		// Set active render graph program
		m_pRenderer->SetRenderGraph(m_pProgram);
	};

	void OnUpdate(Poly::Timestamp dt) override
	{
		pCamera->Update(dt);
		glm::mat4 camMatrix = pCamera->GetMatrix();
		//m_pCambuffer->TransferData(&camMatrix, sizeof(glm::mat4), 0);
		m_pProgram->UpdateGraphResource("camera", sizeof(glm::mat4), &camMatrix);
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

	}
};

Poly::Application* Poly::CreateApplication()
{
	return new Sandbox();
}