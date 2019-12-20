#pragma once
#include <windows.h>

extern Poly::Application* Poly::CreateApplication();

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{
	Poly::Application* app = Poly::CreateApplication();

	app->run();

	delete app;

	return 0;
}