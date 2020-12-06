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

		unsigned GetWidth() const;
		unsigned GetHeight() const;

		GLFWwindow* GetNative() const;
		
		static bool IsOpen();

	private:
		unsigned m_Height = 720;
		unsigned m_Width = 1280;
		std::string m_Title = "";
		GLFWwindow* m_pWindow = nullptr;
		static bool s_Open;

		// Callbacks
		static void CloseWindowCallback(GLFWwindow* pWindow);
		static void KeyCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mods);
		static void MouseMoveCallback(GLFWwindow* pWindow, double x, double y);
	};
}