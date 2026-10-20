#include "Poly.h"
#include "Poly/Core/Input/InputManager.h"
#include "Poly/Core/Logger.h"
#include "Poly/Core/RenderAPI.h"
#include "Poly/Core/Window.h"
#include "Poly/Events/MouseEvent.h"
#include "Poly/ImGui/ImGuiLayer.h"
#include "Poly/RenderGraph/RenderGraph.h"
#include "Poly/RenderGraph/ResourceManager.h"
#include "Poly/Rendering/Renderer.h"
#include "Poly/Resources/AssetHandler.h"
#include "Poly/Resources/AssetTypes/SceneAsset.h"
#include "Poly/World/Entity.h"
#include "Poly/World/Serialization/WorldSerializer.h"
#include "Poly/World/Systems/CameraSystem.h"
#include "Poly/World/Systems/LightSystem.h"
#include "Poly/World/Systems/RenderSystem.h"
#include "Poly/World/World.h"

#include <imgui/imgui.h>

class TestLayer : public Poly::Layer
{
public:
	void OnAttach() override
	{
		m_pCamera = new Poly::Camera();
		m_pCamera->SetMouseSense(2.f);
		m_pCamera->SetMovementSpeed(1.f);
		m_pCamera->SetSprintSpeed(5.f);

		m_World.AddSystem<Poly::RenderSystem>(Poly::World::Phase::PostUpdate);
		m_World.AddSystem<Poly::LightSystem>(Poly::World::Phase::PostUpdate);
		m_World.AddSystem<Poly::CameraSystem>(Poly::World::Phase::PostUpdate);

		// auto sponzaHandle = Poly::AssetHandler::Load<Poly::SceneAsset>("assets/models/sponza/gltf/sponza.gltf");
		// m_World.Instantiate(sponzaHandle);

		Poly::WorldSerializer serializer;
		// serializer.Save(m_World, "assets/worlds/TestWorld.polyworld");
		serializer.Load(m_World, "assets/worlds/TestWorld.polyworld");

		Poly::Renderer*             pRenderer = Poly::Application::Get().GetRenderer();
		Poly::RenderGraph           graph;
		Poly::Ref<Poly::RenderPlan> pPlan = graph.LoadRenderPlan("assets/render/Game.polyplan");
		pRenderer->SetRenderPlan(pPlan);

		// Renders the world to the window, the UI is drawn on top of it
		m_pView = pRenderer->CreateView({.DebugName = "Game", .Stage = SCENE_STAGE_NAME, .pWorld = &m_World});
	}

	void OnUpdate(Poly::Timestamp dt) override
	{
		m_pCamera->Update(dt);

		m_World.Update();

		// Either the primary camera of the world or the free camera, which is not part of the world
		const std::optional<Poly::RenderCamera>& worldCamera = m_World.GetSystem<Poly::CameraSystem>()->GetPrimaryCamera();
		const bool                               useWorld    = m_UseWorldCamera && worldCamera.has_value();
		m_pView->SetCamera(useWorld ? *worldCamera : m_pCamera->GetRenderCamera());
		Poly::Application::Get().GetRenderer()->SubmitView(m_pView);

		DrawUI();
	}

	void OnDetach() override
	{
		m_pView.reset();
		delete m_pCamera;
	}

	void OnEvent(Poly::Event& event) override
	{
		Poly::EventDispatcher eventDispatcher(event);
		eventDispatcher.Dispatch<Poly::Events::MouseButtonPressed>([this](Poly::Events::MouseButtonPressed& event) { return MouseButtonCallback(event.GetButton(), true); });
		eventDispatcher.Dispatch<Poly::Events::MouseButtonReleased>([this](Poly::Events::MouseButtonReleased& event) { return MouseButtonCallback(event.GetButton(), false); });
	}

private:
	void DrawUI()
	{
		// TODO: replace with real UI content; demo window only proves font-only text/widget rendering works.
		ImGui::ShowDemoWindow();

		if (ImGui::Begin("Camera"))
		{
			const bool hasWorldCamera = m_World.GetSystem<Poly::CameraSystem>()->GetPrimaryCamera().has_value();
			ImGui::BeginDisabled(!hasWorldCamera);
			ImGui::Checkbox("Use world camera", &m_UseWorldCamera);
			ImGui::EndDisabled();
			if (!hasWorldCamera)
				ImGui::TextUnformatted("The world has no primary camera");
		}
		ImGui::End();

		// Temp
		const auto& textures = Poly::ResourceManager::GetAllTextures();
		if (ImGui::Begin("Textures"))
		{
			ImGui::BeginChild("TextureList", ImVec2(0, 0), ImGuiChildFlags_Borders);
			for (const auto& textureInfo : textures)
			{
				const char* name = textureInfo.DebugName.empty() ? "(unnamed)" : textureInfo.DebugName.c_str();
				ImGui::SeparatorText(name);
				ImGui::Text("Handle: %u  (%ux%u)", textureInfo.Handle.Get(), textureInfo.Width, textureInfo.Height);
				ImGui::Image(textureInfo.Handle.Get(), ImVec2(128, 128));
			}
			ImGui::EndChild();
		}
		ImGui::End();
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

	static constexpr const char* SCENE_STAGE_NAME = "Scene";

	Poly::Camera*               m_pCamera = nullptr;
	Poly::World                 m_World{"TestWorld"};
	Poly::Ref<Poly::RenderView> m_pView;

	bool m_UseWorldCamera = false;
};

class SandboxApp : public Poly::Application
{
public:
	SandboxApp() {}

	void OnInit() override { PushLayer(new TestLayer()); }

private:
	std::optional<Poly::Window::Properties> GetWindowProperties() const override { return Poly::Window::Properties{1280, 720, "Test App"}; }
};

Poly::Application* Poly::CreateApplication()
{
	return new SandboxApp();
}
