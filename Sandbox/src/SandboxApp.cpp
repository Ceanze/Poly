#include "Poly.h"
#include "Poly/Scene/Scene.h"
#include "Poly/Scene/Entity.h"
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
#include "Poly/Core/Window.h"

#include <imgui/imgui.h>
#include "Poly/Core/Input/InputManager.h"

#include "Poly/Scene/SceneSerializer.h"

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
		glm::vec4 Color = {1.0, 0.0, 1.0, 1.0};
		glm::vec4 Position = {0.0, 1.0, -1.0, 1.0};
	};

	struct LightBuffer
	{
		glm::vec4 LightCount = {1.0, 0.0, 0.0, 0.0};
		PointLight PointLight = {};
	};

	TestLayer()
	{
		m_pWindow = Poly::Window::Create(1280, 720, "Test Window");
		pCamera = new Poly::Camera();
		pCamera->SetAspect(static_cast<float>(m_pWindow->GetWidth()) / m_pWindow->GetHeight());
		pCamera->SetMouseSense(2.f);
		pCamera->SetMovementSpeed(1.f);
		pCamera->SetSprintSpeed(5.f);
		m_pWindow->AddWindowResizeCallback([this](int width, int height) {pCamera->SetAspect(static_cast<float>(width) / height); });

		// Creation
		m_pRenderer = Poly::Renderer::Create(m_pWindow.get());
		m_pGraph = Poly::RenderGraph::Create("TestGraph");
		Poly::Ref<Poly::Pass> pPass = Poly::PBRPass::Create();
		Poly::Ref<Poly::Pass> pImGuiPass = Poly::ImGuiPass::Create();
		m_pScene = Poly::Scene::Create();

		// External resources
		m_pGraph->AddExternalResource({ "camera" }, sizeof(CameraBuffer), Poly::FBufferUsage::UNIFORM_BUFFER);
		m_pGraph->AddExternalResource({ "lights" }, sizeof(LightBuffer), Poly::FBufferUsage::STORAGE_BUFFER);
		m_pGraph->AddExternalResource(m_pScene->GetResourceGroup());

		// Passes and links
		m_pGraph->AddPass(pPass, "pbrPass");
		m_pGraph->AddLink({ "$.camera" }, { "pbrPass.camera" });
		m_pGraph->AddLink({ "$.lights" }, { "pbrPass.lights" });

		// Scene inputs
		m_pGraph->AddLink({ "$.scene:albedoTex" }, { "pbrPass.albedoTex" });
		m_pGraph->AddLink({ "$.scene:metallicTex" }, { "pbrPass.metallicTex" });
		m_pGraph->AddLink({ "$.scene:normalTex" }, { "pbrPass.normalTex" });
		m_pGraph->AddLink({ "$.scene:roughnessTex" }, { "pbrPass.roughnessTex" });
		m_pGraph->AddLink({ "$.scene:aoTex" }, { "pbrPass.aoTex" });
		m_pGraph->AddLink({ "$.scene:combinedTex" }, { "pbrPass.combinedTex" });
		m_pGraph->AddLink({ "$.scene:vertices" }, { "pbrPass.vertices" });
		m_pGraph->AddLink({ "$.scene:instance" }, { "pbrPass.instances" });
		m_pGraph->AddLink({ "$.scene:material" }, { "pbrPass.materialProps" });

		m_pGraph->AddPass(pImGuiPass, "ImGuiPass");
		m_pGraph->AddLink({ "pbrPass.out_Color" }, { "ImGuiPass.fColor" });
		m_pGraph->MarkOutput({ "ImGuiPass.fColor" });

		// Compile
		m_pProgram = m_pGraph->Compile();

		LightBuffer data = {};
		m_pProgram->UpdateGraphResource({ "lights" }, sizeof(LightBuffer), &data);

		//m_pProgram->UpdateGraphResource({ "$.scene:instance" }, sizeof(LightBuffer), &data);

		m_pProgram->SetScene(m_pScene);

		//Poly::SceneSerializer sceneSerializer(m_pScene);
		//sceneSerializer.Deserialize("CubeScene.polyscene");

		 Poly::Entity cubeEntity = m_pScene->CreateEntity();
		// Poly::ResourceManager::ImportAndLoadModel("models/Cube/Cube.gltf", cubeEntity);
		Poly::ResourceManager::ImportAndLoadModel("models/sponza/gltf/sponza.gltf", cubeEntity);

		// Set active render graph program
		m_pRenderer->SetRenderGraph(m_pProgram);

		// TODO REMOVE - NOT HAVE IT HERE
		ImGui::GetIO().DisplaySize = ImVec2(1280, 720);
		m_pWindow->AddWindowResizeCallback([this](int width, int height) { ImGui::GetIO().DisplaySize = ImVec2(width, height); });
	};

	void OnUpdate(Poly::Timestamp dt) override
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(Poly::InputManager::GetMouseX(), Poly::InputManager::GetMouseY());
		io.MouseDown[0] = Poly::InputManager::IsKeyDown(Poly::EKey::MOUSE_LEFT);
		io.MouseDown[1] = Poly::InputManager::IsKeyDown(Poly::EKey::MOUSE_RIGHT);
		io.MouseWheel = Poly::InputManager::GetScrollDeltaY();
		io.MouseWheelH = Poly::InputManager::GetScrollDeltaX();

		if (Poly::InputManager::IsKeyPressed(Poly::EKey::MOUSE_RIGHT))
			m_pWindow->SetMouseMode(Poly::EMouseMode::DISABLED);
		else if (Poly::InputManager::IsKeyReleased(Poly::EKey::MOUSE_RIGHT))
			m_pWindow->SetMouseMode(Poly::EMouseMode::NORMAL);

		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		m_pScene->Update();

		pCamera->Update(dt);
		CameraBuffer data = {pCamera->GetMatrix(), pCamera->GetPosition()};
		m_pProgram->UpdateGraphResource({ "camera" }, sizeof(CameraBuffer), &data);
		m_pRenderer->Render();
	};

	void OnDetach() override
	{
		delete pCamera;
	}

private:
	Poly::Unique<Poly::Window> m_pWindow = nullptr;
	Poly::Camera* pCamera = nullptr;
	Poly::Ref<Poly::Scene> m_pScene = nullptr;
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