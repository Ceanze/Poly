//#ifndef UNICODE
//#define UNICODE
//#endif 

#include "polypch.h"
#include "Core/Window.h"

//LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{
	Poly::Window window(800, 300, "Poly");
	window.createConsole();

	Poly::Logger::init();

	POLY_CORE_INFO("Hello test!");

	while (window.processMessages())
	{

	}

	return 0;
}