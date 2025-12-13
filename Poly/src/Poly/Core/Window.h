#pragma once

#include <polypch.h>
#include "PolyID.h"

struct GLFWwindow;

/**
 * TODO: Make this class platform-independent
 * Currently this class is dependent on GLFWWindow with all its functions and variables
 * As with all "public" classes they should not use any platform specific function/members
 */

namespace Poly
{
	enum class EMouseMode
	{
		NORMAL,
		HIDDEN,
		DISABLED
	};

	class Window
	{
	public:
		struct Properties
		{
			int Height = 0;
			int Width = 0;
			int PosX = 0;
			int PosY = 0;
		};

	public:
		Window(int width, int height, const std::string& name);
		~Window();
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		static Unique<Window> Create(int width, int height, const std::string& name);

		void ToggleBorderlessFullscreen(bool enable);
		void ToggleExclusiveFullscreen(bool enable);

		void SetMouseMode(EMouseMode mouseMode);

		unsigned GetWidth() const;
		unsigned GetHeight() const;

		PolyID GetID() const;

		GLFWwindow* GetNative() const;

		void AddWindowResizeCallback(std::function<void(int width, int height)>&& callback);

	private:
		PolyID m_ID;
		Properties m_CurrentProperties;
		Properties m_SavedProperties;
		std::string m_Title = "";
		GLFWwindow* m_pWindow = nullptr;
		std::vector<std::function<void(int, int)>> m_ResizeCallbacks;

		void SetFullscreen(GLFWwindow* window, bool enable, bool exclusive);

		// Callbacks
		static void CloseWindowCallback(GLFWwindow* pGLFWWindow);
		static void KeyCallback(GLFWwindow* pGLFWWindow, int key, int scancode, int action, int mods);
		static void MouseMoveCallback(GLFWwindow* pGLFWWindow, double x, double y);
		static void MouseButtonCallback(GLFWwindow* pGLFWWindow, int button, int action, int mods);
		static void MouseScrollCallback(GLFWwindow* pGLFWWindow, double x, double y);
		static void FrameBufferSizeCallback(GLFWwindow* pGLFWWindow, int width, int height);
		static void WindowPosCallback(GLFWwindow* pGLFWWindow, int posX, int posY);
	};
}