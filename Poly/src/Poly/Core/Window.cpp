#include "polypch.h"
#include "Window.h"
#include "Poly/Events/EventBus.h"
#include "Poly/Core/Input.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Poly {

	bool Window::s_Open = false;

	Window::Window(int width, int height, const std::string& title)
		: m_Width(width), m_Height(height), m_Title(title)
	{
		// Create window and init glfw
		if (!glfwInit())
			POLY_CORE_FATAL("GLFW could not be initalized!");

		// Tell GLFW not to make an OpenGL context
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// Disable window resize until vulkan renderer can handle it
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_pWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
		if (!m_pWindow) {
			glfwTerminate();
			POLY_CORE_FATAL("Could not create a GLFW window!");
		}

		glfwMakeContextCurrent(m_pWindow);
		s_Open = true;

		// Set callbacks
		glfwSetWindowCloseCallback(m_pWindow, CloseWindowCallback);
		glfwSetKeyCallback(m_pWindow, KeyCallback);
		glfwSetCursorPosCallback(m_pWindow, MouseMoveCallback);
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

	bool Window::IsOpen()
	{
		return false;
	}

	void Window::CloseWindowCallback(GLFWwindow* pWindow)
	{
		CloseWindowEvent e = {};
		POLY_EVENT_PUB(e);
	}

	void Window::KeyCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE)
			s_Open = false;

		if (action == GLFW_PRESS)
			Input::SetKeyPressed(key);
		else if (action == GLFW_RELEASE)
			Input::SetKeyReleased(key);
	}

	void Window::MouseMoveCallback(GLFWwindow* pWindow, double x, double y)
	{
		// Only record mouse movement when toggled
		if (Input::IsKeyToggled(GLFW_KEY_C)) {
			int width, height;
			glfwGetWindowSize(pWindow, &width, &height);
			glfwSetCursorPos(pWindow, (double)width * 0.5, (double)height * 0.5);
			glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			// Send cursor offset from centre of window
			Input::SetMouseDelta(x - (double)width * 0.5, y - (double)height * 0.5);
		}
		else {
			glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			Input::SetMouseDelta(0.0, 0.0);
		}
		Input::SetMousePosition(x, y);
	}

}