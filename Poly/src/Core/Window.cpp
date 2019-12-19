#include "polypch.h"
#include "Window.h"

#include <stdio.h>
#include <io.h>
#include <fcntl.h>

std::wstring Poly::Window::WindowClass::wndClassName = L"Default Window";
Poly::Window::WindowClass Poly::Window::WindowClass::wndClass;

namespace Poly {

	std::wstring& Window::WindowClass::getName()
	{
		return wndClassName;
	}

	HINSTANCE Window::WindowClass::getInstance()
	{
		return wndClass.hInstance;
	}

	Window::WindowClass::WindowClass() :
		hInstance(GetModuleHandle(NULL))
	{
		// Create WNDCLASS, which is a set of data, and not a C++ class
		WNDCLASS wc = { };

		// Must set important values
		wc.lpfnWndProc = handleEventsSetup; // Seems to be setting a callback function for window events
		wc.hInstance = hInstance; // Set this WNDCLASS to be the instance main takes in
		wc.lpszClassName = getName().c_str(); // String that identifies the WNDCLASS

		// Register WNDCLASS to the operating system
		RegisterClass(&wc);
	}

	Window::WindowClass::~WindowClass()
	{
		UnregisterClass(wndClassName.c_str(), getInstance());
	}

	Window::Window(int width, int height, const std::string& name)
	{
		m_width = width;
		m_height = height;

		std::wstring wname = std::wstring(name.begin(), name.end());

		RECT clientSize;
		clientSize.left = 0;
		clientSize.right = width;
		clientSize.top = 0;
		clientSize.bottom = height;
		AdjustWindowRect(&clientSize, WS_OVERLAPPEDWINDOW, false);

		// Create the window.
		m_window = CreateWindowEx(
			0,                              // Optional window styles.
			WindowClass::getName().c_str(), // Window class (Maps to the one we created before this with the same CLASS_NAME)
			wname.c_str(),    // Window text
			WS_OVERLAPPEDWINDOW,            // Window style

			// Position and size
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

			NULL,       // Parent window    
			NULL,       // Menu
			WindowClass::getInstance(),  // Instance handle
			this        // Additional application data
		);

		if (m_window == NULL)
		{
			POLY_CORE_ERROR(GetLastError());
			return;
		}

		ShowWindow(m_window, SW_SHOWDEFAULT);
	}

	Window::~Window()
	{
		DestroyWindow(m_window);

		if (!GetConsoleWindow())
			destoryConsole();
	}

	bool Window::processMessages()
	{
		MSG msg;
		ZeroMemory(&msg, sizeof(MSG));

		if (PeekMessage(&msg, m_window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If window was closed
		if (msg.message == WM_NULL && !IsWindow(m_window))
		{
			DestroyWindow(m_window);
			return false;
		}
		
		return true;
	}

	void Window::createConsole()
	{
		AllocConsole();

		SetConsoleTitleW(L"Poly debug console");
		FILE* stream;
		freopen_s(&stream, "conin$", "r", stdin);
		freopen_s(&stream, "conout$", "w", stdout);
		freopen_s(&stream, "conout$", "w", stderr);
	}

	void Window::destoryConsole()
	{
		FreeConsole();
	}

	LRESULT CALLBACK Window::handleEventsSetup(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_NCCREATE)
		{
			// Get window pointer from window creation data
			const CREATESTRUCTW* const create = reinterpret_cast<CREATESTRUCTW*>(lParam);
			Window* const wnd = static_cast<Window*>(create->lpCreateParams);

			// Set user data to store pointer to window class
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(wnd));

			// Set message procedure to normal (no longer setup)
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::handleEvents));

			// Forward message to window class handler
			return wnd->handleEvents(hWnd, uMsg, wParam, lParam);
		}
		// If it wasn't the crete msg, call the default function
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	LRESULT CALLBACK Window::handleEvents(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

			EndPaint(hWnd, &ps);
			return 0;
		}

		case WM_SIZE:
		{
			int width = LOWORD(lParam);
			int height = HIWORD(lParam);
			return 0;
		}

		case WM_KEYDOWN:
		{
			POLY_CORE_TRACE(wParam);
		}
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

}