#include "ImGuiLayer.h"

#include "Poly/Core/Application.h"
#include "Poly/Events/KeyEvent.h"
#include "Poly/Events/MouseEvent.h"
#include "Poly/Events/WindowEvent.h"

#include <imgui.h>

namespace Poly
{
	ImGuiLayer::ImGuiLayer()
	{
		ImGui::CreateContext();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		Window* pWindow            = Application::Get().GetWindow();
		ImGui::GetIO().DisplaySize = ImVec2(pWindow->GetWidth(), pWindow->GetHeight());
	}

	ImGuiLayer::~ImGuiLayer()
	{
		ImGui::DestroyContext();
	}

	void ImGuiLayer::BeginFrame()
	{
		ImGui::NewFrame();
	}

	void ImGuiLayer::OnEvent(Event& event)
	{
		ImGuiIO& io = ImGui::GetIO();

		EventDispatcher eventDispatcher(event);
		eventDispatcher.Dispatch<Events::MouseMoved>([this](auto& event) { return OnMouseMoved(event); });
		eventDispatcher.Dispatch<Events::MouseButtonPressed>([this](auto& event) { return OnMouseButtonPressed(event); });
		eventDispatcher.Dispatch<Events::MouseButtonReleased>([this](auto& event) { return OnMouseButtonReleased(event); });
		eventDispatcher.Dispatch<Events::MouseScrolled>([this](auto& event) { return OnMouseScrolled(event); });
		eventDispatcher.Dispatch<Events::KeyPressed>([this](auto& event) { return OnKeyPressed(event); });
		eventDispatcher.Dispatch<Events::KeyReleased>([this](auto& event) { return OnKeyReleased(event); });
		eventDispatcher.Dispatch<Events::WindowResized>([this](auto& event) { return OnWindowResized(event); });
	}

	bool ImGuiLayer::OnMouseMoved(Events::MouseMoved& event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(static_cast<float>(event.GetX()), static_cast<float>(event.GetY()));

		return false;
	}

	bool ImGuiLayer::OnMouseButtonPressed(Events::MouseButtonPressed& event)
	{
		ImGuiIO& io = ImGui::GetIO();

		if (!io.WantCaptureMouse)
			return false;

		if (event.GetButton() == EKey::MOUSE_LEFT)
			io.MouseDown[0] = true;
		else if (event.GetButton() == EKey::MOUSE_RIGHT)
			io.MouseDown[1] = true;

		return true;
	}

	bool ImGuiLayer::OnMouseButtonReleased(Events::MouseButtonReleased& event)
	{
		ImGuiIO& io = ImGui::GetIO();

		if (!io.WantCaptureMouse)
			return false;

		if (event.GetButton() == EKey::MOUSE_LEFT)
			io.MouseDown[0] = false;
		else if (event.GetButton() == EKey::MOUSE_RIGHT)
			io.MouseDown[1] = false;

		return true;
	}

	bool ImGuiLayer::OnMouseScrolled(Events::MouseScrolled& event)
	{
		ImGuiIO& io = ImGui::GetIO();

		if (!io.WantCaptureMouse)
			return false;

		io.MouseWheel  = static_cast<float>(event.GetDeltaY());
		io.MouseWheelH = static_cast<float>(event.GetDeltaX());

		return true;
	}

	bool ImGuiLayer::OnKeyPressed(Events::KeyPressed& event)
	{
		ImGuiIO& io = ImGui::GetIO();

		if (!io.WantCaptureKeyboard)
			return false;

		// Not implemented yet

		return false;
	}

	bool ImGuiLayer::OnKeyReleased(Events::KeyReleased& event)
	{
		ImGuiIO& io = ImGui::GetIO();

		if (!io.WantCaptureKeyboard)
			return false;

		// Not implemented yet

		return false;
	}

	bool ImGuiLayer::OnWindowResized(Events::WindowResized& event)
	{
		ImGui::GetIO().DisplaySize = ImVec2(static_cast<float>(event.GetWidth()), static_cast<float>(event.GetHeight()));

		return false;
	}
} // namespace Poly
