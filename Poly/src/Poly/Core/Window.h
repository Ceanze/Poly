#pragma once

#include <polypch.h>

namespace Poly {

	class Window
	{
	private:
		class WindowClass
		{
		public:
			static std::wstring& getName();
			static HINSTANCE getInstance();

		private:
			WindowClass();
			~WindowClass();
			WindowClass(const WindowClass&) = delete;
			WindowClass& operator=(const WindowClass&) = delete;

			static std::wstring wndClassName;
			static WindowClass wndClass;
			HINSTANCE hInstance;
		};


	public:
		Window(int width, int height, const std::string& name);
		~Window();
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		bool processMessages();
		void createConsole();
		void destoryConsole();

	private:
		// Window procedure to handle events
		static LRESULT CALLBACK handleEventsSetup(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		//static LRESULT CALLBACK handleEventsThunk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK handleEvents(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		int m_height, m_width;
		HWND m_window;
	};

}