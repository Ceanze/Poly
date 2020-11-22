#pragma once
#include <windows.h>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "Engine.h"

extern Poly::Application* Poly::CreateApplication();

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Poly::Application* app = Poly::CreateApplication();

	app->Run();

	delete app;

	return 0;
}