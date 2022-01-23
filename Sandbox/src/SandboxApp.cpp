#include "Poly.h"
#include "Poly/Rendering/Scene.h"
#include "Poly/Rendering/Renderer.h"
#include "Poly/Rendering/RenderGraph/RenderGraphCompiler.h"
#include "Poly/Rendering/RenderGraph/RenderGraphProgram.h"
#include "Poly/Rendering/RenderGraph/RenderGraph.h"
#include "Poly/Rendering/RenderGraph/Passes/ImGuiPass.h"
#include "Poly/Rendering/RenderGraph/Passes/PBRPass.h"
#include "Poly/Core/RenderAPI.h"
#include "Poly/Rendering/RenderGraph/Resource.h"
#include "Platform/API/TextureView.h"
#include "Platform/API/Texture.h"
#include "Platform/API/Buffer.h"
#include "Poly/Resources/ResourceManager.h"

#include <imgui/imgui.h>
#include "Poly/Core/Input/Input.h"

class TestLayer : public Poly::Layer
{
public:
	struct CameraBuffer
	{
		glm::mat4 Mat;
		glm::vec4 Pos;
	};

	struct PointLight
	{
		glm::vec4 Color = {1.0, 1.0, 1.0, 1.0};
		glm::vec4 Position = {0.0, 1.0, -1.0, 1.0};
	};

	struct LightBuffer
	{
		glm::vec4 LightCount = {1.0, 0.0, 0.0, 0.0};
		PointLight PointLight = {};
	};

	TestLayer()
	{
		pCamera = new Poly::Camera();
		pCamera->SetAspect(1280.f / 720.f);
		pCamera->SetMouseSense(2.f);
		pCamera->SetMovementSpeed(1.f);
		pCamera->SetSprintSpeed(5.f);

		// Creation
		m_pRenderer = Poly::Renderer::Create();
		m_pGraph = Poly::RenderGraph::Create("TestGraph");
		Poly::Ref<Poly::Pass> pPass = Poly::PBRPass::Create();
		Poly::Ref<Poly::Pass> pImGuiPass = Poly::ImGuiPass::Create();

		// External resources
		m_pGraph->AddExternalResource("camera", sizeof(CameraBuffer), Poly::FBufferUsage::UNIFORM_BUFFER);
		m_pGraph->AddExternalResource("lights", sizeof(LightBuffer), Poly::FBufferUsage::STORAGE_BUFFER);

		// Passes and links
		m_pGraph->AddPass(pPass, "pbrPass");
		m_pGraph->AddLink("$.camera", "pbrPass.camera");
		m_pGraph->AddLink("$.lights", "pbrPass.lights");

		m_pGraph->AddPass(pImGuiPass, "ImGuiPass");
		m_pGraph->AddLink("pbrPass.out", "ImGuiPass.out");
		m_pGraph->MarkOutput("ImGuiPass.out");

		// Compile
		m_pProgram = m_pGraph->Compile();

		LightBuffer data = {};
		m_pProgram->UpdateGraphResource("lights", sizeof(LightBuffer), &data);

		Poly::Ref<Poly::Scene> pScene = Poly::Scene::Create();
		m_pProgram->SetScene(pScene);

		PolyID sponza = Poly::ResourceManager::LoadModel("models/sponza/glTF/Sponza.gltf");
		//PolyID cube = Poly::ResourceManager::LoadModel("models/Cube/Cube.gltf");
		//PolyID helmet = Poly::ResourceManager::LoadModel("models/FlightHelmet/FlightHelmet.gltf");
		pScene->AddModel(sponza);
		//pScene->AddModel(cube);
		//pScene->AddModel(helmet);

		// Set active render graph program
		m_pRenderer->SetRenderGraph(m_pProgram);


		// TODO REMOVE - NOT HAVE IT HERE
		ImGui::GetIO().DisplaySize = ImVec2(1280, 720);
	};

	void OnUpdate(Poly::Timestamp dt) override
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(Poly::Input::GetMousePosition().x, Poly::Input::GetMousePosition().y);
		io.MouseDown[0] = Poly::Input::IsKeyPressed(Poly::KeyCode(Poly::EKey::MOUSE_LEFT));
		io.MouseDown[1] = Poly::Input::IsKeyPressed(Poly::KeyCode(Poly::EKey::MOUSE_RIGHT));

		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		pCamera->Update(dt);
		CameraBuffer data = {pCamera->GetMatrix(), pCamera->GetPosition()};
		m_pProgram->UpdateGraphResource("camera", sizeof(CameraBuffer), &data);
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