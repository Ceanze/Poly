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

		GLFWwindow* getNativeWindow() const;

	private:
		unsigned height = 720;
		unsigned width = 1280;
		std::string title;
		GLFWwindow* window = nullptr;

		// Callbacks
		static void closeWindowCallback(GLFWwindow* w);
	};
}