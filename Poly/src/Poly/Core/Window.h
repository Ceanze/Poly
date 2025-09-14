#pragma once

#include <polypch.h>

struct GLFWwindow;

/**
 * TODO: Make this class platform-independent
 * Currently this class is dependent on GLFWWindow with all its functions and variables
 * As with all "public" classes they should not use any platform specific function/members
 */

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

		void AddWindowResizeCallback(std::function<void(int width, int height)>&& callback);

	private:
		unsigned m_Height = 720;
		unsigned m_Width = 1280;
		std::string m_Title = "";
		GLFWwindow* m_pWindow = nullptr;
		std::vector<std::function<void(int, int)>> m_ResizeCallbacks;

		// Callbacks
		static void CloseWindowCallback(GLFWwindow* pWindow);
		static void KeyCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mods);
		static void MouseMoveCallback(GLFWwindow* pWindow, double x, double y);
		static void MouseButtonCallback(GLFWwindow* pWindow, int button, int action, int mods);
		static void FrameBufferSizeCallback(GLFWwindow* pWindow, int width, int height);
	};
}