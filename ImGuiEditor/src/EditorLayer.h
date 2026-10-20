#pragma once

#include "EditorState.h"
#include "Panels/HierarchyPanel.h"
#include "Panels/InspectorPanel.h"

#include <Poly/API/EngineContext.h>
#include <Poly/Core/Camera.h>
#include <Poly/Core/Layer.h>
#include <Poly/Events/MouseEvent.h>
#include <Poly/RenderGraph/ResourceManager.h>
#include <Poly/Rendering/RenderView.h>

namespace Editor
{
	/*
	 * The editor: owns the API context holding the world, renders the world and draws the editor UI.
	 * The UI only ever goes through the API, the world itself is only touched here to render it.
	 * The world is rendered to textures shown in two panels: the viewport, seen from the free camera of the editor,
	 * and the game view, seen from the primary camera of the world.
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
		// A panel showing the world, rendered by its own view to a texture following the size of the panel
		struct Viewport
		{
			const char*                 pName = "";
			Poly::Ref<Poly::RenderView> pView;
			Poly::TextureHandle         Texture;
			uint32                      Width   = 0;
			uint32                      Height  = 0;
			bool                        Visible = false;
		};

		/**
		 * Draws the panel of a viewport
		 * @param viewport - viewport to draw
		 * @param pPlaceholder - text to show instead of the rendered world, nullptr to show the world
		 */
		void DrawViewport(Viewport& viewport, const char* pPlaceholder = nullptr);
		void ResizeViewport(Viewport& viewport, uint32 width, uint32 height);
		void SubmitViewport(Viewport& viewport, const Poly::RenderCamera& camera);
		void DestroyViewport(Viewport& viewport);
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

		Poly::Camera* m_pCamera = nullptr;

		// Seen from m_pCamera
		Viewport m_EditorViewport{.pName = "Viewport"};

		// Seen from the primary camera of the world, shows nothing while the world has none
		Viewport m_GameViewport{.pName = "Game"};
	};
} // namespace Editor
