#include "polypch.h"
#include "Window.h"
#include "Poly/Events/EventBus.h"
#include "Poly/Core/Input/Input.h"
#include "Platform/GLFW/GLFWTypes.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Poly
{
	Window::Window(int width, int height, const std::string& title)
		: m_Title(title)
	{
		// Create window and init glfw
		if (!glfwInit())
			POLY_CORE_FATAL("GLFW could not be initalized!");

		// Tell GLFW not to make an OpenGL context
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_pWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
		if (!m_pWindow) {
			glfwTerminate();
			POLY_CORE_FATAL("Could not create a GLFW window!");
		}

		// TODO: Investigate if this data should be separate from the initial window creation size. This should only differ however
		//		 on HDPI screens, such as Retina.
		int newWidth, newHeight;
		glfwGetFramebufferSize(m_pWindow, &newWidth, &newHeight);
		m_CurrentProperties.Width = newWidth;
		m_CurrentProperties.Height = newHeight;

		glfwMakeContextCurrent(m_pWindow);

		glfwSetWindowUserPointer(m_pWindow, this);

		// Set callbacks
		glfwSetWindowCloseCallback(m_pWindow, CloseWindowCallback);
		glfwSetKeyCallback(m_pWindow, KeyCallback);
		glfwSetCursorPosCallback(m_pWindow, MouseMoveCallback);
		glfwSetMouseButtonCallback(m_pWindow, MouseButtonCallback);
		glfwSetFramebufferSizeCallback(m_pWindow, FrameBufferSizeCallback);
		glfwSetWindowPosCallback(m_pWindow, WindowPosCallback);
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_pWindow);
		glfwTerminate();
	}

	void Window::ToggleBorderlessFullscreen(bool enable)
	{
		SetFullscreen(m_pWindow, enable, false);
	}

	void Window::ToggleExclusiveFullscreen(bool enable)
	{
		SetFullscreen(m_pWindow, enable, true);
	}

	unsigned Window::GetWidth() const
	{
		return m_CurrentProperties.Width;
	}

	unsigned Window::GetHeight() const
	{
		return m_CurrentProperties.Height;
	}

	GLFWwindow* Window::GetNative() const
	{
		return m_pWindow;
	}

	void Window::AddWindowResizeCallback(std::function<void(int width, int height)>&& callback)
	{
		m_ResizeCallbacks.emplace_back(std::move(callback));
	}

	void Window::SetFullscreen(GLFWwindow* pGLFWWindow, bool enable, bool exclusive)
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		Properties& props = m_SavedProperties;

		if (enable) {
			// Save current window position/size
			glfwGetWindowPos(pGLFWWindow, &props.PosX, &props.PosY);
			glfwGetWindowSize(pGLFWWindow, &props.Width, &props.Height);

			// Remove window borders/decorations
			glfwSetWindowAttrib(pGLFWWindow, GLFW_DECORATED, GLFW_FALSE);

			// Resize to cover the monitor
			if (exclusive)
				glfwSetWindowMonitor(pGLFWWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
			else
				glfwSetWindowMonitor(pGLFWWindow, nullptr, 0, 0, mode->width, mode->height, 0);
		}
		else {
			// Restore decorations
			glfwSetWindowAttrib(pGLFWWindow, GLFW_DECORATED, GLFW_TRUE);

			// Restore original window position/size
			glfwSetWindowMonitor(pGLFWWindow, nullptr, props.PosX, props.PosY, props.Width, props.Height, 0);
		}
	}

	void Window::CloseWindowCallback(GLFWwindow* pGLFWWindow)
	{
		CloseWindowEvent e = {};
		POLY_EVENT_PUB(e);
	}

	void Window::KeyCallback(GLFWwindow* pGLFWWindow, int key, int scancode, int action, int mods)
	{
		EKey polyKey = ConvertToPolyKey(key);
		FKeyModifier polyMod = ConvertToPolyModifier(mods);
		KeyCode keyCode(polyKey, polyMod);
		if (action == GLFW_PRESS)
			Input::SetKeyPressed(keyCode);
		else if (action == GLFW_RELEASE)
			Input::SetKeyReleased(keyCode);
	}

	void Window::MouseMoveCallback(GLFWwindow* pGLFWWindow, double x, double y)
	{
		// TODO: This function should only update the necessary values!

		// Only record mouse movement when toggled
		if (Input::IsKeyToggled(KeyCode(EKey::C))) {
			int width, height;
			glfwGetWindowSize(pGLFWWindow, &width, &height);
			glfwSetCursorPos(pGLFWWindow, static_cast<double>(width) * 0.5, static_cast<double>(height) * 0.5);
			glfwSetInputMode(pGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			// Send cursor offset from centre of window
			Input::SetMouseDelta(x - static_cast<double>(width) * 0.5, y - static_cast<double>(height) * 0.5);
		}
		else {
			glfwSetInputMode(pGLFWWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			Input::SetMouseDelta(0.0, 0.0);
		}
		Input::SetMousePosition(x, y);
	}

	void Window::MouseButtonCallback(GLFWwindow* pGLFWWindow, int button, int action, int mods)
	{
		EKey polyKey = ConvertToPolyKey(button);
		FKeyModifier polyMod = ConvertToPolyModifier(mods);
		KeyCode keyCode(polyKey, polyMod);
		if (action == GLFW_PRESS)
			Input::SetKeyPressed(keyCode);
		else if (action == GLFW_RELEASE)
			Input::SetKeyReleased(keyCode);
	}

	void Window::FrameBufferSizeCallback(GLFWwindow* pGLFWWindow, int width, int height)
	{
		Window* pWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(pGLFWWindow));
		
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(pGLFWWindow, &width, &height);
			glfwWaitEvents();
		}

		pWindow->m_CurrentProperties.Width = width;
		pWindow->m_CurrentProperties.Height = height;

		for (const auto& callback : pWindow->m_ResizeCallbacks)
			callback(width, height);
	}

	void Window::WindowPosCallback(GLFWwindow* pGLFWWindow, int posX, int posY)
	{
		Window* pWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(pGLFWWindow));
		
		pWindow->m_CurrentProperties.PosX = posX;
		pWindow->m_CurrentProperties.PosY = posY;
	}
}
