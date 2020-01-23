#include "polypch.h"
#include "Window.h"
#include "Poly/Events/EventBus.h"

#include <GLFW/glfw3.h>

namespace Poly {

	Window::Window(int width, int height, const std::string& title)
		: width(width), height(height), title(title)
	{
		// Create window and init glfw
		if (!glfwInit())
			POLY_CORE_FATAL("GLFW could not be initalized!");
		
		this->window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
		if (!this->window) {
			glfwTerminate();
			POLY_CORE_FATAL("Could not create a GLFW window!");
		}

		glfwMakeContextCurrent(this->window);

		// Set callbacks
		glfwSetWindowCloseCallback(this->window, closeWindowCallback);
	}

	Window::~Window()
	{
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

	void Window::closeWindowCallback(GLFWwindow* w)
	{
		EventBus::get().publish(&CloseWindowEvent());
	}

}