#pragma once

#include <polypch.h>

struct GLFWwindow;

namespace Poly {

	class Window
	{
	public:
		Window(int width, int height, const std::string& name);
		~Window();
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		unsigned getWidth() const;
		unsigned getHeight() const;

		GLFWwindow* getNative() const;
		
		static bool isOpen();

	private:
		unsigned height = 720;
		unsigned width = 1280;
		std::string title;
		GLFWwindow* window = nullptr;
		static bool open;

		// Callbacks
		static void closeWindowCallback(GLFWwindow* w);
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouseMoveCallback(GLFWwindow* window, double x, double y);
	};
}