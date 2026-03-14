#include "Poly.h"
#include "Poly/Scene/Scene.h"
#include "Poly/Scene/Entity.h"
#include "Poly/Rendering/Renderer.h"
#include "Poly/Rendering/RenderGraph/RenderGraphCompiler.h"
#include "Poly/Rendering/RenderGraph/RenderGraphProgram.h"
#include "Poly/Rendering/RenderGraph/RenderGraph.h"
#include "Poly/Rendering/RenderGraph/Passes/ImGuiPass.h"
#include "Poly/Rendering/RenderGraph/Passes/PBRPass.h"
#include "Poly/Rendering/RenderGraph/Resource.h"
#include "Platform/API/Buffer.h"
#include "Poly/Resources/ResourceManager.h"
#include "Poly/Core/Window.h"
#include "Poly/Events/WindowEvent.h"
#include "Poly/Events/MouseEvent.h"

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

	TestLayer() {}

	void OnAttach() override
	{
		Poly::Window* pWindow = Poly::Application::Get().GetWindow();

		pCamera = new Poly::Camera();
		pCamera->SetAspect(static_cast<float>(pWindow->GetWidth()) / pWindow->GetHeight());
		pCamera->SetMouseSense(2.f);
		pCamera->SetMovementSpeed(1.f);
		pCamera->SetSprintSpeed(5.f);

		// Creation
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

		m_TextureID = Poly::ResourceManager::ImportAndLoadTexture("textures/ceanze.png", Poly::EFormat::R8G8B8A8_UNORM);
		m_pTextureView = Poly::ResourceManager::GetTextureView(m_TextureID);

		// Set active render graph program
		Poly::Application::Get().GetRenderer()->SetRenderGraph(m_pProgram);
	}

	void OnUpdate(Poly::Timestamp dt) override
	{
		ImGui::ShowDemoWindow();

		ImGui::Begin("Vulkan Texture Test");
		if (m_pTextureView)
		{
			ImGui::Text("Texture View: %p", m_pTextureView);
			ImGui::Image((ImTextureID)m_pTextureView, ImVec2(256, 256));
		}

		ImGui::End();

		m_pScene->Update();

		pCamera->Update(dt);
		CameraBuffer data = {pCamera->GetMatrix(), pCamera->GetPosition()};
		m_pProgram->UpdateGraphResource({ "camera" }, sizeof(CameraBuffer), &data);
	};

	void OnDetach() override
	{
		delete pCamera;
	}

	void OnEvent(Poly::Event& event) override
	{
		Poly::EventDispatcher eventDispatcher(event);
		eventDispatcher.Dispatch<Poly::Events::WindowResized>([this](Poly::Events::WindowResized& event) { return WindowResizeCallback(event); });
		eventDispatcher.Dispatch<Poly::Events::MouseButtonPressed>([this](Poly::Events::MouseButtonPressed& event) { return MouseButtonCallback(event.GetButton(), true); });
		eventDispatcher.Dispatch<Poly::Events::MouseButtonReleased>([this](Poly::Events::MouseButtonReleased& event) { return MouseButtonCallback(event.GetButton(), false); });
	}

	bool WindowResizeCallback(Poly::Events::WindowResized& event)
	{
		pCamera->SetAspect(static_cast<float>(event.GetWidth()) / event.GetHeight());

		return true;
	}

	bool MouseButtonCallback(Poly::EKey button, bool pressed)
	{
		Poly::Window* pWindow = Poly::Application::Get().GetWindow();

		if (button == Poly::EKey::MOUSE_RIGHT && pressed)
			pWindow->SetMouseMode(Poly::EMouseMode::DISABLED);
		else if (button == Poly::EKey::MOUSE_RIGHT && !pressed)
			pWindow->SetMouseMode(Poly::EMouseMode::NORMAL);

		return true;
	}

private:
	Poly::Camera* pCamera = nullptr;
	Poly::Ref<Poly::Scene> m_pScene = nullptr;
	Poly::Ref<Poly::Buffer> m_pCambuffer = nullptr;
	Poly::Ref<Poly::RenderGraph> m_pGraph = nullptr;
	Poly::Ref<Poly::RenderGraphProgram> m_pProgram = nullptr;

	Poly::PolyID m_TextureID = Poly::PolyID::None();
	Poly::TextureView* m_pTextureView = nullptr;
};

class Sandbox : public Poly::Application
{
public:
	Sandbox() {}

	void OnInit() override
	{
		PushLayer(new TestLayer());
	}

private:
	std::optional<Poly::Window::Properties> GetWindowProperties() const override { return Poly::Window::Properties{ 1280, 720, "Poly Engine" }; }

};

Poly::Application* Poly::CreateApplication()
{
	return new Sandbox();
}
