#include "polypch.h"
#include "Engine.h"

#include "Timer.h"
#include "Window.h"
#include "RenderAPI.h"
#include "Timestamp.h"
#include "Application.h"

#include "Poly/Resources/ResourceLoader.h"
#include "Poly/Resources/ResourceManager.h"

#include <GLFW/glfw3.h>

#define FIXED_UPDATE_FREQ 60.0

namespace Poly
{
	void Engine::Init()
	{
		Poly::Logger::init();

		s_pWindow = new Window(1280, 720, "Test Window");

		RenderAPI::Init(RenderAPI::BackendAPI::VULKAN, s_pWindow);

		ResourceLoader::Init();
		ResourceManager::Init();
	}

	void Engine::Run(Application* pApp)
	{
		Timer timer;
		Timestamp timeCollector;
		Timestamp fixedTime = Timestamp::FromSeconds(1.0 / FIXED_UPDATE_FREQ);

		while (pApp->IsRunning())
		{
			Timestamp dt = timer.GetDeltaTime();
			timeCollector += dt;

			// Every frame
			{
				glfwPollEvents();
				pApp->Update(dt);
			}

			// Every FIXED_UPDATE_FREQ frame
			if (timeCollector >= fixedTime)
			{
				timeCollector -= fixedTime;
				pApp->FixedUpdate(dt);
			}

			timer.Tick();
		}
	}

	void Engine::Release()
	{
		ResourceLoader::Release();
		ResourceManager::Release();

		delete s_pWindow;

		RenderAPI::Release();
	}
}