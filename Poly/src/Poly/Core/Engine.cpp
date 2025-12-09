#include "polypch.h"
#include "Engine.h"

#include "Timer.h"
#include "RenderAPI.h"
#include "Timestamp.h"
#include "Application.h"

#include "Poly/Core/Input/InputManager.h"
#include "Poly/Resources/ResourceLoader.h"
#include "Poly/Resources/ResourceManager.h"
#include "Poly/Resources/Shader/ShaderManager.h"

#include <GLFW/glfw3.h>

#define FIXED_UPDATE_FREQ 60.0

namespace Poly
{
	void Engine::Init()
	{
		Poly::Logger::init();

		if (!glfwInit())
		{
			POLY_CORE_FATAL("GLFW could not be initalized!");
			return;
		}

		RenderAPI::Init(RenderAPI::BackendAPI::VULKAN);

		ShaderManager::Init();
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
				InputManager::Update();

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
		ShaderManager::Release();
		ResourceLoader::Release();
		ResourceManager::Release();
		RenderAPI::Release();
		glfwTerminate();
	}
}