#pragma once

#include "Poly/Core/Layer.h"
#include "Poly/RenderGraph/ResourceManager.h"

namespace Poly
{
	class RenderCatalog;

	namespace Events
	{
		class MouseMoved;
		class MouseButtonPressed;
		class MouseButtonReleased;
		class MouseScrolled;
		class KeyPressed;
		class KeyReleased;
		class WindowResized;
	} // namespace Events

	/*
	 * Owns the ImGui context and everything needed to render it: the font atlas, the vertex/index/globals buffers
	 * and the "ui" render graph feature. Layers only issue ImGui:: calls in their OnUpdate().
	 *
	 * Usage:
	 *
	 *   // Before building the render program
	 *   Application::Get().GetImGuiLayer()->RegisterRenderFeature(catalog);
	 *   graph.Begin().AddFeature(ImGuiLayer::FEATURE_NAME)...Build();
	 */
	class ImGuiLayer : public Layer
	{
	public:
		static constexpr const char* FEATURE_NAME = "ui";

		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach() override;
		void OnEvent(Event& event) override;

		/*
		 * Starts a new ImGui frame, called by the application before any layer updates.
		 */
		void BeginFrame();

		/*
		 * Finalizes this frame's ImGui draw data and uploads it to the UI buffers, called by the application
		 * after all layer updates and before the renderer records the frame.
		 */
		void EndFrame();

		/*
		 * Registers the UI resources, the "ui" pass and the FEATURE_NAME feature in the catalog.
		 * @param catalog - catalog the render program will be built from
		 */
		void RegisterRenderFeature(RenderCatalog& catalog);

	private:
		bool OnMouseMoved(Events::MouseMoved& event);
		bool OnMouseButtonPressed(Events::MouseButtonPressed& event);
		bool OnMouseButtonReleased(Events::MouseButtonReleased& event);
		bool OnMouseScrolled(Events::MouseScrolled& event);
		bool OnKeyPressed(Events::KeyPressed& event);
		bool OnKeyReleased(Events::KeyReleased& event);
		bool OnWindowResized(Events::WindowResized& event);

		TextureHandle m_FontTextureHandle;
		SamplerHandle m_FontSamplerHandle;

		BufferHandle m_GlobalsBufferHandle;
		BufferHandle m_VertexBufferHandle;
		BufferHandle m_IndexBufferHandle;
	};
} // namespace Poly
