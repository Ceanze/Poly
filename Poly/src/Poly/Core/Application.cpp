#include "polypch.h"
#include "Application.h"
#include "GLFW/glfw3.h"
#include "Poly/Events/EventBus.h"

namespace Poly
{

	Application::Application()
	{
		Poly::Logger::init();

		this->renderer = Renderer::create(Renderer::BACKEND::VULKAN);
		this->renderer->initialize();


		POLY_CORE_INFO("Application created!");

		POLY_EVENT_SUB(Application, onCloseWindowEvent);
	}

	Application::~Application()
	{
		POLY_EVENT_UNSUB(Application, onCloseWindowEvent);
		this->renderer->shutdown();
	}

	void Application::run()
	{
		while (this->running)
		{
			glfwPollEvents();

			for (auto layer : this->layerStack)
				layer->onUpdate();
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