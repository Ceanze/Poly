#pragma once
#ifdef POLY_WINDOWS
#include <windows.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <stdlib.h>

#include "Engine.h"

extern Poly::Application* Poly::CreateApplication();

int main()
{
#ifdef POLY_WINDOWS
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	Poly::Engine::Init();

	Poly::Application* app = Poly::CreateApplication();

	Poly::Engine::Run(app);

	delete app;

	Poly::Engine::Release();

	return 0;
}
