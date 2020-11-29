#include "polypch.h"
#include "Engine.h"

#include "Application.h"
#include "Timestamp.h"
#include "Timer.h"

#include "Poly/Resources/ResourceLoader.h"

#include <GLFW/glfw3.h>

#define FIXED_UPDATE_FREQ 60.0

namespace Poly
{
	void Engine::Init()
	{
		ResourceLoader::Init();
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
		}
	}

	void Engine::Release()
	{
		ResourceLoader::Release();
	}
}