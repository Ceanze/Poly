#pragma once

#include "EditorState.h"
#include "Panels/HierarchyPanel.h"
#include "Panels/InspectorPanel.h"

#include <Poly/API/EngineContext.h>
#include <Poly/Core/Camera.h>
#include <Poly/Core/Layer.h>
#include <Poly/Events/MouseEvent.h>
#include <Poly/Events/WindowEvent.h>
#include <Poly/RenderGraph/RenderGraph.h>
#include <Poly/RenderGraph/RenderResourceTable.h>
#include <Poly/RenderGraph/ResourceManager.h>

namespace Editor
{
	/*
	 * The editor: owns the API context holding the world, renders the world and draws the editor UI.
	 * The UI only ever goes through the API, the world itself is only touched here to render it.
	 */
	class EditorLayer : public Poly::Layer
	{
	public:
		/**
		 * @param worldPath - VFS path of a world to open at startup, empty to start with an empty world
		 */
		explicit EditorLayer(std::string_view worldPath = "");

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(Poly::Timestamp dt) override;
		void OnEvent(Poly::Event& event) override;

	private:
		void RegisterGeometryFeature();
		void DrawMenuBar();
		void DrawPathPopup();
		void HandleShortcuts();
		void Save();

		void OnApiEvent(const Poly::API::ApiEvent& event);

		bool OnWindowResized(Poly::Events::WindowResized& event);
		bool OnMouseButton(Poly::EKey button, bool pressed);

		Poly::API::EngineContext          m_Context{"Untitled"};
		Poly::API::EventBus::Subscription m_EventSubscription;

		EditorState    m_State;
		HierarchyPanel m_HierarchyPanel;
		InspectorPanel m_InspectorPanel;

		enum class EPathPopup
		{
			None,
			Open,
			SaveAs,
		};

		EPathPopup  m_PathPopup       = EPathPopup::None;
		bool        m_OpenPathPopup   = false;
		char        m_PathBuffer[512] = "assets/worlds/TestWorld.polyworld";
		std::string m_LastError;
		std::string m_StartupWorldPath;

		Poly::Camera*                  m_pCamera  = nullptr;
		Poly::Ref<Poly::RenderCatalog> m_pCatalog = Poly::CreateRef<Poly::RenderCatalog>();
		Poly::RenderGraph              m_Graph{m_pCatalog};
		Poly::RenderResourceTable      m_ViewResources;

		Poly::BufferHandle m_CameraBufferHandle;
		Poly::BufferHandle m_LightsBufferHandle;
	};
} // namespace Editor
