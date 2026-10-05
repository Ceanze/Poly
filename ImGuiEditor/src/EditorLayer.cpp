#include "EditorLayer.h"

#include <Platform/API/CommandBuffer.h>
#include <Poly/Core/Application.h>
#include <Poly/Core/Window.h>
#include <Poly/ImGui/ImGuiLayer.h>
#include <Poly/RenderGraph/ExecuteContext.h>
#include <Poly/RenderGraph/Feature/FeaturePort.h>
#include <Poly/RenderGraph/RenderCatalog.h>
#include <Poly/Rendering/Renderer.h>
#include <Poly/World/Systems/RenderSystem.h>

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

namespace Editor
{
	EditorLayer::EditorLayer(std::string_view worldPath)
	    : m_StartupWorldPath(worldPath)
	{}

	void EditorLayer::OnAttach()
	{
		Poly::Window* pWindow = Poly::Application::Get().GetWindow();

		m_pCamera = new Poly::Camera();
		m_pCamera->SetAspect(static_cast<float>(pWindow->GetWidth()) / pWindow->GetHeight());
		m_pCamera->SetMouseSense(2.f);
		m_pCamera->SetMovementSpeed(1.f);
		m_pCamera->SetSprintSpeed(5.f);

		// Registers the scene resources in the catalog, so it has to be added before the render program is built
		m_Context.GetWorld().AddSystem<Poly::RenderSystem>(Poly::World::Phase::PostUpdate, *m_pCatalog);

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

		m_EventSubscription = m_Context.Events().Subscribe([this](const Poly::API::ApiEvent& event) { OnApiEvent(event); });

		if (!m_StartupWorldPath.empty())
		{
			if (const Poly::Result<void> result = m_Context.Worlds().Load(m_StartupWorldPath); !result)
				m_LastError = result.error().Message;
		}
	}

	void EditorLayer::OnDetach()
	{
		delete m_pCamera;
	}

	void EditorLayer::OnUpdate(Poly::Timestamp dt)
	{
		// The UI makes its changes through the API first, then the world is updated with them and rendered
		HandleShortcuts();
		DrawMenuBar();
		DrawPathPopup();
		m_HierarchyPanel.Draw(m_Context, m_State);
		m_InspectorPanel.Draw(m_Context, m_State);

		m_Context.Tick();
		Poly::Application::Get().GetRenderer()->Submit({.pWorld         = &m_Context.GetWorld(),
		                                                .pViewResources = &m_ViewResources});

		m_pCamera->Update(dt);
		CameraBuffer cameraData = {m_pCamera->GetMatrix(), m_pCamera->GetPosition()};
		Poly::ResourceManager::UploadBufferData(m_CameraBufferHandle, &cameraData, sizeof(CameraBuffer));
	}

	void EditorLayer::OnEvent(Poly::Event& event)
	{
		Poly::EventDispatcher eventDispatcher(event);
		eventDispatcher.Dispatch<Poly::Events::WindowResized>([this](Poly::Events::WindowResized& event) { return OnWindowResized(event); });
		eventDispatcher.Dispatch<Poly::Events::MouseButtonPressed>([this](Poly::Events::MouseButtonPressed& event) { return OnMouseButton(event.GetButton(), true); });
		eventDispatcher.Dispatch<Poly::Events::MouseButtonReleased>([this](Poly::Events::MouseButtonReleased& event) { return OnMouseButton(event.GetButton(), false); });
	}

	// Same geometry pass as the Sandbox, see the notes there about the order of the MapGlobal() calls
	void EditorLayer::RegisterGeometryFeature()
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
		    .WithGraphicsPipeline()
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
			    // An empty world has nothing to draw, and no geometry might have been loaded yet for the index buffer to exist
			    if (!pRenderSystem || pRenderSystem->GetDrawBatches().empty())
				    return;

			    Poly::CommandBuffer* pCmd = ctx.GetCommandBuffer();
			    pCmd->BindIndexBuffer(pRenderSystem->GetIndexBuffer(), 0, Poly::EIndexType::UINT32);
			    for (const Poly::DrawBatch& batch : pRenderSystem->GetDrawBatches())
				    pCmd->DrawIndexedInstanced(batch.IndexCount, batch.InstanceCount, batch.BaseIndex, batch.BaseVertex, batch.FirstInstance);
		    });

		m_Graph.RegisterFeature("geometry").WithPass("pbr");
	}

	void EditorLayer::DrawMenuBar()
	{
		if (!ImGui::BeginMainMenuBar())
			return;

		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New"))
				m_Context.Worlds().New();

			if (ImGui::MenuItem("Open..."))
			{
				m_PathPopup     = EPathPopup::Open;
				m_OpenPathPopup = true;
			}

			if (ImGui::MenuItem("Save", "Ctrl+S"))
				Save();

			if (ImGui::MenuItem("Save As..."))
			{
				m_PathPopup     = EPathPopup::SaveAs;
				m_OpenPathPopup = true;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "Ctrl+Z", false, m_Context.History().CanUndo()))
				m_Context.History().Undo();

			if (ImGui::MenuItem("Redo", "Ctrl+Y", false, m_Context.History().CanRedo()))
				m_Context.History().Redo();

			ImGui::EndMenu();
		}

		ImGui::Separator();
		ImGui::Text("%s%s", m_Context.Worlds().GetName().c_str(), m_Context.Worlds().IsDirty() ? "*" : "");

		if (!m_LastError.empty())
		{
			ImGui::Separator();
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", m_LastError.c_str());
		}

		ImGui::EndMainMenuBar();
	}

	// TODO: Replace with a file dialog
	void EditorLayer::DrawPathPopup()
	{
		const char* pTitle = m_PathPopup == EPathPopup::Open ? "Open World" : "Save World As";

		if (m_OpenPathPopup)
		{
			ImGui::OpenPopup(pTitle);
			m_OpenPathPopup = false;
		}

		if (!ImGui::BeginPopupModal(pTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		ImGui::InputText("VFS path", m_PathBuffer, sizeof(m_PathBuffer));

		if (ImGui::Button("OK"))
		{
			const Poly::Result<void> result = m_PathPopup == EPathPopup::Open ? m_Context.Worlds().Load(m_PathBuffer) : m_Context.Worlds().Save(m_PathBuffer);
			m_LastError                     = result ? "" : result.error().Message;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}

	void EditorLayer::HandleShortcuts()
	{
		// Typing in a text field has its own undo and delete
		if (ImGui::GetIO().WantTextInput)
			return;

		if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Z))
			m_Context.History().Undo();

		if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Y) || ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z))
			m_Context.History().Redo();

		if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S))
			Save();

		if (ImGui::IsKeyPressed(ImGuiKey_Delete) && m_State.Selection != Poly::PolyID::None())
			m_Context.Entities().Destroy(m_State.Selection);
	}

	void EditorLayer::Save()
	{
		// A world that has never been saved has no path yet
		if (m_Context.Worlds().GetPath().empty())
		{
			m_PathPopup     = EPathPopup::SaveAs;
			m_OpenPathPopup = true;
			return;
		}

		const Poly::Result<void> result = m_Context.Worlds().Save();
		m_LastError                     = result ? "" : result.error().Message;
	}

	void EditorLayer::OnApiEvent(const Poly::API::ApiEvent& event)
	{
		// The selection can be destroyed by anything, including an undo or a newly loaded world
		if (!m_Context.Entities().Exists(m_State.Selection))
			m_State.Selection = Poly::PolyID::None();
	}

	bool EditorLayer::OnWindowResized(Poly::Events::WindowResized& event)
	{
		m_pCamera->SetAspect(static_cast<float>(event.GetWidth()) / event.GetHeight());
		return true;
	}

	bool EditorLayer::OnMouseButton(Poly::EKey button, bool pressed)
	{
		Poly::Window* pWindow = Poly::Application::Get().GetWindow();

		if (button == Poly::EKey::MOUSE_RIGHT && pressed)
			pWindow->SetMouseMode(Poly::EMouseMode::DISABLED);
		else if (button == Poly::EKey::MOUSE_RIGHT && !pressed)
			pWindow->SetMouseMode(Poly::EMouseMode::NORMAL);

		return true;
	}
} // namespace Editor
