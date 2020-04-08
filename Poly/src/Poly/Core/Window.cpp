#include "polypch.h"
#include "Window.h"
#include "Poly/Events/EventBus.h"
#include "Poly/Core/Input.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Poly {

	bool Window::open = false;

	Window::Window(int width, int height, const std::string& title)
		: width(width), height(height), title(title)
	{
		// Create window and init glfw
		if (!glfwInit())
			POLY_CORE_FATAL("GLFW could not be initalized!");

		// Tell GLFW not to make an OpenGL context
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// Disable window resize until vulkan renderer can handle it
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		
		this->window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
		if (!this->window) {
			glfwTerminate();
			POLY_CORE_FATAL("Could not create a GLFW window!");
		}

		glfwMakeContextCurrent(this->window);
		open = true;

		// Set callbacks
		glfwSetWindowCloseCallback(this->window, closeWindowCallback);
		glfwSetKeyCallback(this->window, keyCallback);
		glfwSetCursorPosCallback(this->window, mouseMoveCallback);
	}

	Window::~Window()
	{
		glfwDestroyWindow(this->window);
		glfwTerminate();
	}

	unsigned Window::getWidth() const
	{
		return this->width;
	}

	unsigned Window::getHeight() const
	{
		return this->height;
	}

	GLFWwindow* Window::getNativeWindow() const
	{
		return this->window;
	}

	bool Window::isOpen()
	{
		return false;
	}

	void Window::closeWindowCallback(GLFWwindow* w)
	{
		POLY_EVENT_PUB(CloseWindowEvent());
	}

	void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE)
			open = false;

		if (action == GLFW_PRESS)
			Input::setKeyPressed(key);
		else if (action == GLFW_RELEASE)
			Input::setKeyReleased(key);
	}

	void Window::mouseMoveCallback(GLFWwindow* window, double x, double y)
	{
		// Only record mouse movement when toggled
		if (Input::isKeyToggled(GLFW_KEY_C)) {
			int width, height;
			glfwGetWindowSize(window, &width, &height);
			glfwSetCursorPos(window, (double)width * 0.5, (double)height * 0.5);
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			// Send cursor offset from centre of window
			Input::setMouseDelta(x - (double)width * 0.5, y - (double)height * 0.5);
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			Input::setMouseDelta(0.0, 0.0);
		}
	}

}