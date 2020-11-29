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

	Poly::Engine::Init();

	Poly::Application* app = Poly::CreateApplication();

	Poly::Engine::Run(app);

	delete app;

	Poly::Engine::Release();

	return 0;
}