#pragma once
#include <windows.h>

extern Poly::Application* Poly::CreateApplication();

int main()
{
	Poly::Application* app = Poly::CreateApplication();

	app->run();

	delete app;

	return 0;
}