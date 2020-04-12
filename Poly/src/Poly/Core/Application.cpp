#include "polypch.h"
#include "Application.h"
#include "GLFW/glfw3.h"
#include "Poly/Events/EventBus.h"
#include "Window.h"
#include "RendererAPI.h"

namespace Poly
{

	Application::Application()
	{
		Poly::Logger::init();

		POLY_EVENT_SUB(Application, onCloseWindowEvent);
	}

	Application::~Application()
	{
		POLY_EVENT_UNSUB(Application, onCloseWindowEvent);
	}

	void Application::run()
	{
		static auto currTime = std::chrono::high_resolution_clock::now();
		static float dt = 0.f;

		while (this->running)
		{
			dt = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - currTime).count();
			currTime = std::chrono::high_resolution_clock::now();

			glfwPollEvents();

			for (auto layer : this->layerStack)
				layer->onUpdate(dt);
		}
	}

	void Application::pushLayer(Layer* layer)
	{
		this->layerStack.pushLayer(layer);
		layer->onAttach();
	}

	void Application::pushOverlay(Layer* layer)
	{
		this->layerStack.pushOverlay(layer);
		layer->onAttach();
	}

	void Application::onCloseWindowEvent(CloseWindowEvent* e)
	{
		this->running = false;
	}
}