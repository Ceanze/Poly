#include "Platform/API/CommandBuffer.h"
#include "Poly.h"
#include "Poly/Core/Input/InputManager.h"
#include "Poly/Core/Logger.h"
#include "Poly/Core/RenderAPI.h"
#include "Poly/Core/Window.h"
#include "Poly/Events/MouseEvent.h"
#include "Poly/Events/WindowEvent.h"
#include "Poly/ImGui/ImGuiLayer.h"
#include "Poly/RenderGraph/ExecuteContext.h"
#include "Poly/RenderGraph/Feature/FeaturePort.h"
#include "Poly/RenderGraph/RenderCatalog.h"
#include "Poly/RenderGraph/RenderGraph.h"
#include "Poly/RenderGraph/RenderResourceTable.h"
#include "Poly/RenderGraph/ResourceManager.h"
#include "Poly/Rendering/Renderer.h"
#include "Poly/Resources/AssetHandler.h"
#include "Poly/Resources/AssetTypes/SceneAsset.h"
#include "Poly/Scene/Entity.h"
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

		auto sponzaHandle = Poly::AssetHandler::Load<Poly::SceneAsset>("assets/models/sponza/gltf/sponza.gltf");
		m_World.Instantiate(sponzaHandle);

		RegisterGeometryFeature();
		Poly::Application::Get().GetImGuiLayer()->RegisterRenderFeature(*m_pCatalog);

		Poly::Ref<Poly::RenderProgram> pProgram = m_Graph.Begin()
		                                              .AddFeature("geometry")
		                                              .AddFeature(Poly::ImGuiLayer::FEATURE_NAME)
		                                              .WithFinalState(Poly::ToSemanticName(Poly::EFeaturePort::Color), Poly::FResourceState::Present)
		                                              .Build();

		Poly::Application::Get().GetRenderer()->SetRenderProgram(pProgram);

		m_CameraBufferHandle = Poly::ResourceManager::CreateUniformBuffer(sizeof(CameraBuffer), "Camera");
		m_LightsBufferHandle = Poly::ResourceManager::CreateStorageBuffer(sizeof(LightBuffer), Poly::EMemoryUsage::CPU_VISIBLE, "Lights");

		LightBuffer lights = {};
		Poly::ResourceManager::UploadBufferData(m_LightsBufferHandle, &lights, sizeof(LightBuffer));

		m_ViewResources.Set("Camera", m_CameraBufferHandle);
		m_ViewResources.Set("Lights", m_LightsBufferHandle);
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
	// Scene resources (scene.*) are registered by the RenderSystem.
	// Until slang, the shader's bufferAddresses[] slots are assigned in MapGlobal() call order, so the order of the
	// MapGlobal() calls below is load bearing: Camera(0), scene.vertices(1), scene.instances(2), Lights(3), scene.materials(4).
	void RegisterGeometryFeature()
	{
		m_Graph.RegisterResource("Camera").WithType(Poly::EResourceType::UniformBuffer);
		m_Graph.RegisterResource("Lights").WithType(Poly::EResourceType::StorageBuffer);

		m_Graph.RegisterPass("pbr")
		    .WithShader("assets/shaders/pbr_bindless.vert", Poly::FShaderStage::VERTEX)
		    .WithShader("assets/shaders/pbr_bindless.frag", Poly::FShaderStage::FRAGMENT)
		    .MapResource(Poly::EFeaturePort::Color, "out_Color")
		    .MapResource(Poly::EFeaturePort::Depth, "depth")
		    .MapGlobal("Camera", "camera")
		    .MapGlobal(Poly::RenderSystem::VERTICES_RESOURCE_NAME, "vertices")
		    .MapGlobal(Poly::RenderSystem::INSTANCE_RESOURCE_NAME, "instances")
		    .MapGlobal("Lights", "lights")
		    .MapGlobal(Poly::RenderSystem::MATERIAL_RESOURCE_NAME, "materialProps")
		    .WithGraphicsPipeline() // TODO: add a default pipeline to the graph so this can be omitted and the default used
		    .Topology(Poly::ETopology::TRIANGLE_LIST)
		    .PolygonMode(Poly::EPolygonMode::FILL)
		    .CullMode(Poly::ECullMode::BACK)
		    .ClockwiseFrontFace(false)
		    .DepthTestEnable(true)
		    .DepthWriteEnable(true)
		    .DepthCompareOp(Poly::ECompareOp::LESS_OR_EQUAL)
		    .AddColorBlendAttachment()
		    .BlendEnable(false)
		    .ColorWriteMask(Poly::FColorComponentFlag::RED | Poly::FColorComponentFlag::GREEN | Poly::FColorComponentFlag::BLUE |
			                Poly::FColorComponentFlag::ALPHA)
		    .FinishColorBlendAttachment()
		    .FinishPipeline()
		    .WithExecuteFn([](Poly::ExecuteContext& ctx) {
			    const Poly::World*        pWorld        = ctx.GetWorld();
			    const Poly::RenderSystem* pRenderSystem = pWorld ? pWorld->GetSystem<Poly::RenderSystem>() : nullptr;
			    if (!pRenderSystem)
				    return;

			    Poly::CommandBuffer* pCmd = ctx.GetCommandBuffer();
			    pCmd->BindIndexBuffer(pRenderSystem->GetIndexBuffer(), 0, Poly::EIndexType::UINT32);
			    for (const Poly::DrawBatch& batch : pRenderSystem->GetDrawBatches())
				    pCmd->DrawIndexedInstanced(batch.IndexCount, batch.InstanceCount, batch.BaseIndex, batch.BaseVertex, batch.FirstInstance);
		    });

		m_Graph.RegisterFeature("geometry").WithPass("pbr");
	}

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
