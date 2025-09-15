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
		: m_Width(width), m_Height(height), m_Title(title)
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
		m_Width = newWidth;
		m_Height = newHeight;

		glfwMakeContextCurrent(m_pWindow);

		glfwSetWindowUserPointer(m_pWindow, this);

		// Set callbacks
		glfwSetWindowCloseCallback(m_pWindow, CloseWindowCallback);
		glfwSetKeyCallback(m_pWindow, KeyCallback);
		glfwSetCursorPosCallback(m_pWindow, MouseMoveCallback);
		glfwSetMouseButtonCallback(m_pWindow, MouseButtonCallback);
		glfwSetFramebufferSizeCallback(m_pWindow, FrameBufferSizeCallback);
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_pWindow);
		glfwTerminate();
	}

	unsigned Window::GetWidth() const
	{
		return m_Width;
	}

	unsigned Window::GetHeight() const
	{
		return m_Height;
	}

	GLFWwindow* Window::GetNative() const
	{
		return m_pWindow;
	}

	void Window::AddWindowResizeCallback(std::function<void(int width, int height)>&& callback)
	{
		m_ResizeCallbacks.emplace_back(std::move(callback));
	}

	void Window::CloseWindowCallback(GLFWwindow* pWindow)
	{
		CloseWindowEvent e = {};
		POLY_EVENT_PUB(e);
	}

	void Window::KeyCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mods)
	{
		EKey polyKey = ConvertToPolyKey(key);
		FKeyModifier polyMod = ConvertToPolyModifier(mods);
		KeyCode keyCode(polyKey, polyMod);
		if (action == GLFW_PRESS)
			Input::SetKeyPressed(keyCode);
		else if (action == GLFW_RELEASE)
			Input::SetKeyReleased(keyCode);
	}

	void Window::MouseMoveCallback(GLFWwindow* pWindow, double x, double y)
	{
		// TODO: This function should only update the necessary values!

		// Only record mouse movement when toggled
		if (Input::IsKeyToggled(KeyCode(EKey::C))) {
			int width, height;
			glfwGetWindowSize(pWindow, &width, &height);
			glfwSetCursorPos(pWindow, static_cast<double>(width) * 0.5, static_cast<double>(height) * 0.5);
			glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			// Send cursor offset from centre of window
			Input::SetMouseDelta(x - static_cast<double>(width) * 0.5, y - static_cast<double>(height) * 0.5);
		}
		else {
			glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			Input::SetMouseDelta(0.0, 0.0);
		}
		Input::SetMousePosition(x, y);
	}

	void Window::MouseButtonCallback(GLFWwindow* pWindow, int button, int action, int mods)
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

		pWindow->m_Width = width;
		pWindow->m_Height = height;

		for (const auto& callback : pWindow->m_ResizeCallbacks)
			callback(width, height);
	}
}
