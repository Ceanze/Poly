#include "EditorLayer.h"

#include <imgui/imgui.h>

#include <Poly/Core/Application.h>
#include <Poly/Core/Window.h>
#include <Poly/ImGui/ImGuiLayer.h>
#include <Poly/RenderGraph/RenderCatalog.h>
#include <Poly/Rendering/Features/GeometryFeature.h>
#include <Poly/Rendering/Renderer.h>
#include <Poly/World/Systems/LightSystem.h>
#include <Poly/World/Systems/RenderSystem.h>

namespace
{
	constexpr const char* SCENE_STAGE_NAME = "Scene";
} // namespace

namespace Editor
{
	EditorLayer::EditorLayer(std::string_view worldPath)
	    : m_StartupWorldPath(worldPath)
	{}

	void EditorLayer::OnAttach()
	{
		m_pCamera = new Poly::Camera();
		m_pCamera->SetMouseSense(2.f);
		m_pCamera->SetMovementSpeed(1.f);
		m_pCamera->SetSprintSpeed(5.f);

		m_Context.GetWorld().AddSystem<Poly::RenderSystem>(Poly::World::Phase::PostUpdate, *m_pCatalog);
		m_Context.GetWorld().AddSystem<Poly::LightSystem>(Poly::World::Phase::PostUpdate, *m_pCatalog);

		Poly::GeometryFeature::Register(*m_pCatalog);
		Poly::Application::Get().GetImGuiLayer()->RegisterRenderFeature(*m_pCatalog);

		// clang-format off
		Poly::Ref<Poly::RenderPlan> pPlan = m_Graph.BeginRenderPlan()
		                                        .AddViewStage(SCENE_STAGE_NAME)
		                                            .AddFeature(Poly::GeometryFeature::FEATURE_NAME)
		                                        .EndStage()
		                                        .AddFrameFeature(Poly::ImGuiLayer::FEATURE_NAME)
		                                        .Build();
		// clang-format on

		Poly::Renderer* pRenderer = Poly::Application::Get().GetRenderer();
		pRenderer->SetRenderPlan(pPlan);

		// The target is set once the viewport panel knows its size
		m_pView = pRenderer->CreateView({.DebugName = "Editor Viewport", .Stage = SCENE_STAGE_NAME, .pWorld = &m_Context.GetWorld()});

		m_EventSubscription = m_Context.Events().Subscribe([this](const Poly::API::ApiEvent& event) { OnApiEvent(event); });

		if (!m_StartupWorldPath.empty())
		{
			if (const Poly::Result<void> result = m_Context.Worlds().Load(m_StartupWorldPath); !result)
				m_LastError = result.error().Message;
		}
	}

	void EditorLayer::OnDetach()
	{
		m_pView.reset();
		Poly::ResourceManager::Destroy(m_ViewportTexture);

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
		DrawViewport();

		m_pCamera->Update(dt);

		m_Context.Tick();

		// Nothing is rendered of the world while the viewport panel is hidden
		if (m_ViewportVisible)
		{
			m_pView->SetWorld(&m_Context.GetWorld());
			m_pView->SetCamera(m_pCamera->GetRenderCamera());
			Poly::Application::Get().GetRenderer()->SubmitView(m_pView);
		}
	}

	void EditorLayer::OnEvent(Poly::Event& event)
	{
		Poly::EventDispatcher eventDispatcher(event);
		eventDispatcher.Dispatch<Poly::Events::MouseButtonPressed>([this](Poly::Events::MouseButtonPressed& event) { return OnMouseButton(event.GetButton(), true); });
		eventDispatcher.Dispatch<Poly::Events::MouseButtonReleased>([this](Poly::Events::MouseButtonReleased& event) { return OnMouseButton(event.GetButton(), false); });
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

	void EditorLayer::DrawViewport()
	{
		ImGui::SetNextWindowPos(ImVec2(360.0f, 60.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(960.0f, 540.0f), ImGuiCond_FirstUseEver);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		m_ViewportVisible = ImGui::Begin("Viewport");
		if (m_ViewportVisible)
		{
			const ImVec2 available = ImGui::GetContentRegionAvail();
			ResizeViewport(static_cast<uint32>(std::max(available.x, 1.0f)), static_cast<uint32>(std::max(available.y, 1.0f)));

			ImGui::Image(m_ViewportTexture.Get(), ImVec2(static_cast<float>(m_ViewportWidth), static_cast<float>(m_ViewportHeight)));
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void EditorLayer::ResizeViewport(uint32 width, uint32 height)
	{
		if (m_ViewportTexture.IsValid() && width == m_ViewportWidth && height == m_ViewportHeight)
			return;

		// The resource manager keeps the old texture alive for the frames still using it
		Poly::ResourceManager::Destroy(m_ViewportTexture);

		m_ViewportTexture = Poly::ResourceManager::CreateTexture2D(width, height, Poly::EFormat::R8G8B8A8_UNORM,
		                                                           Poly::FTextureUsage::COLOR_ATTACHMENT | Poly::FTextureUsage::SAMPLED, "Editor Viewport");
		m_ViewportWidth   = width;
		m_ViewportHeight  = height;

		m_pView->SetTarget(m_ViewportTexture);
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
