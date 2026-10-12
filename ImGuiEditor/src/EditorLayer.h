#pragma once

#include "EditorState.h"
#include "Panels/HierarchyPanel.h"
#include "Panels/InspectorPanel.h"

#include <Poly/API/EngineContext.h>
#include <Poly/Core/Camera.h>
#include <Poly/Core/Layer.h>
#include <Poly/Events/MouseEvent.h>
#include <Poly/RenderGraph/RenderGraph.h>
#include <Poly/RenderGraph/ResourceManager.h>
#include <Poly/Rendering/RenderView.h>

namespace Editor
{
	/*
	 * The editor: owns the API context holding the world, renders the world and draws the editor UI.
	 * The UI only ever goes through the API, the world itself is only touched here to render it.
	 * The world is rendered to a texture, which is shown in the viewport panel.
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
		void DrawMenuBar();
		void DrawPathPopup();
		void DrawViewport();
		void ResizeViewport(uint32 width, uint32 height);
		void HandleShortcuts();
		void Save();

		void OnApiEvent(const Poly::API::ApiEvent& event);

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

		// The view rendering the world and the texture it renders to, which follows the size of the viewport panel
		Poly::Ref<Poly::RenderView> m_pView;
		Poly::TextureHandle         m_ViewportTexture;
		uint32                      m_ViewportWidth   = 0;
		uint32                      m_ViewportHeight  = 0;
		bool                        m_ViewportVisible = false;
	};
} // namespace Editor
