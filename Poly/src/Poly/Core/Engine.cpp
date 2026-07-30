#include "Engine.h"

#include "Application.h"
#include "Poly/Core/Input/InputManager.h"
#include "Poly/Resources/AssetLoader.h"
#include "Poly/Resources/AssetManager.h"
#include "Poly/Resources/Shader/ShaderManager.h"
#include "polypch.h"
#include "RenderAPI.h"
#include "ThreadPool.h"
#include "Timer.h"
#include "Timestamp.h"

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

		ThreadPool::Init();

		RenderAPI::Init(RenderAPI::BackendAPI::VULKAN);

		ShaderManager::Init();
		AssetLoader::Init();
		AssetManager::Init();
	}

	void Engine::Run(Application* pApp)
	{
		Timer     timer;
		Timestamp timeCollector;
		Timestamp fixedTime = Timestamp::FromSeconds(1.0 / FIXED_UPDATE_FREQ);

		while (pApp->IsRunning())
		{
			Timestamp dt = timer.GetDeltaTime();
			timeCollector += dt;

			// Every frame
			{
				InputManager::Update();

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
		ThreadPool::Release();

		ShaderManager::Release();
		AssetLoader::Release();
		AssetManager::Release();
		RenderAPI::Release();
		glfwTerminate();
	}
} // namespace Poly