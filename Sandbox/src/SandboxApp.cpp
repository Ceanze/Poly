#include "Poly.h"
#include "Poly/Core/Input/InputManager.h"
#include "Poly/Core/Logger.h"
#include "Poly/Core/RenderAPI.h"
#include "Poly/Core/Window.h"
#include "Poly/Events/MouseEvent.h"
#include "Poly/Events/WindowEvent.h"
#include "Poly/ImGui/ImGuiLayer.h"
#include "Poly/RenderGraph/Feature/FeaturePort.h"
#include "Poly/RenderGraph/RenderCatalog.h"
#include "Poly/RenderGraph/RenderGraph.h"
#include "Poly/RenderGraph/RenderResourceTable.h"
#include "Poly/RenderGraph/ResourceManager.h"
#include "Poly/Rendering/Features/GeometryFeature.h"
#include "Poly/Rendering/Renderer.h"
#include "Poly/Resources/AssetHandler.h"
#include "Poly/Resources/AssetTypes/SceneAsset.h"
#include "Poly/Scene/Entity.h"
#include "Poly/World/Serialization/WorldSerializer.h"
#include "Poly/World/Systems/RenderSystem.h"
#include "Poly/World/World.h"

#include <imgui/imgui.h>

namespace
{
	struct CameraBuffer
	{
		glm::mat4 Mat;
		glm::vec4 Pos;
	};

	struct PointLight
	{
		glm::vec4 Color    = {100.0f, 100.0f, 100.0f, 1.0f};
		glm::vec4 Position = {0.0f, 1.0f, -1.0f, 1.0f};
	};

	struct LightBuffer
	{
		glm::vec4  LightCount = {1.0f, 0.0f, 0.0f, 0.0f};
		PointLight PointLight = {};
	};
} // namespace

class TestLayer : public Poly::Layer
{
public:
	void OnAttach() override
	{
		Poly::Window* pWindow = Poly::Application::Get().GetWindow();

		m_pCamera = new Poly::Camera();
		m_pCamera->SetAspect(static_cast<float>(pWindow->GetWidth()) / pWindow->GetHeight());
		m_pCamera->SetMouseSense(2.f);
		m_pCamera->SetMovementSpeed(1.f);
		m_pCamera->SetSprintSpeed(5.f);

		// Registers the scene resources in the catalog, so it has to be added before the render program is built
		m_World.AddSystem<Poly::RenderSystem>(Poly::World::Phase::PostUpdate, *m_pCatalog);

		// auto sponzaHandle = Poly::AssetHandler::Load<Poly::SceneAsset>("assets/models/sponza/gltf/sponza.gltf");
		// m_World.Instantiate(sponzaHandle);

		Poly::WorldSerializer serializer;
		// serializer.Save(m_World, "assets/worlds/TestWorld.polyworld");
		serializer.Load(m_World, "assets/worlds/TestWorld.polyworld");

		Poly::GeometryFeature::Register(*m_pCatalog);
		Poly::Application::Get().GetImGuiLayer()->RegisterRenderFeature(*m_pCatalog);

		Poly::Ref<Poly::RenderProgram> pProgram = m_Graph.Begin()
		                                              .AddFeature(Poly::GeometryFeature::FEATURE_NAME)
		                                              .AddFeature(Poly::ImGuiLayer::FEATURE_NAME)
		                                              .WithFinalState(Poly::ToSemanticName(Poly::EFeaturePort::Color), Poly::FResourceState::Present)
		                                              .Build();

		Poly::Application::Get().GetRenderer()->SetRenderProgram(pProgram);

		m_CameraBufferHandle = Poly::ResourceManager::CreateUniformBuffer(sizeof(CameraBuffer), "Camera");
		m_LightsBufferHandle = Poly::ResourceManager::CreateStorageBuffer(sizeof(LightBuffer), Poly::EMemoryUsage::CPU_VISIBLE, "Lights");

		LightBuffer lights = {};
		Poly::ResourceManager::UploadBufferData(m_LightsBufferHandle, &lights, sizeof(LightBuffer));

		m_ViewResources.Set(Poly::GeometryFeature::CAMERA_RESOURCE_NAME, m_CameraBufferHandle);
		m_ViewResources.Set(Poly::GeometryFeature::LIGHTS_RESOURCE_NAME, m_LightsBufferHandle);
	}

	void OnUpdate(Poly::Timestamp dt) override
	{
		m_World.Update();
		Poly::Application::Get().GetRenderer()->Submit({.pWorld         = &m_World,
		                                                .pViewResources = &m_ViewResources});

		m_pCamera->Update(dt);
		CameraBuffer cameraData = {m_pCamera->GetMatrix(), m_pCamera->GetPosition()};
		Poly::ResourceManager::UploadBufferData(m_CameraBufferHandle, &cameraData, sizeof(CameraBuffer));

		DrawUI();
	}

	void OnDetach() override { delete m_pCamera; }

	void OnEvent(Poly::Event& event) override
	{
		Poly::EventDispatcher eventDispatcher(event);
		eventDispatcher.Dispatch<Poly::Events::WindowResized>([this](Poly::Events::WindowResized& event) { return WindowResizeCallback(event); });
		eventDispatcher.Dispatch<Poly::Events::MouseButtonPressed>([this](Poly::Events::MouseButtonPressed& event) { return MouseButtonCallback(event.GetButton(), true); });
		eventDispatcher.Dispatch<Poly::Events::MouseButtonReleased>([this](Poly::Events::MouseButtonReleased& event) { return MouseButtonCallback(event.GetButton(), false); });
	}

private:
	void DrawUI()
	{
		// TODO: replace with real UI content; demo window only proves font-only text/widget rendering works.
		ImGui::ShowDemoWindow();

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

	bool WindowResizeCallback(Poly::Events::WindowResized& event)
	{
		m_pCamera->SetAspect(static_cast<float>(event.GetWidth()) / event.GetHeight());

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

	Poly::Camera*                  m_pCamera  = nullptr;
	Poly::Ref<Poly::RenderCatalog> m_pCatalog = Poly::CreateRef<Poly::RenderCatalog>();
	Poly::RenderGraph              m_Graph{m_pCatalog};
	Poly::World                    m_World{"TestWorld"};
	Poly::RenderResourceTable      m_ViewResources;

	Poly::BufferHandle m_CameraBufferHandle;
	Poly::BufferHandle m_LightsBufferHandle;
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
