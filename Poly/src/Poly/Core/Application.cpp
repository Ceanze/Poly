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

		RendererAPI::create(RendererAPI::BACKEND::VULKAN);

		POLY_CORE_INFO("Application created!");

		POLY_EVENT_SUB(Application, onCloseWindowEvent);
	}

	Application::~Application()
	{
		POLY_EVENT_UNSUB(Application, onCloseWindowEvent);
		RendererAPI::shutdown();
	}

	void Application::run()
	{
		while (this->running)
		{
			glfwPollEvents();

			for (auto layer : this->layerStack)
				layer->onUpdate();

			RendererAPI::render();
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