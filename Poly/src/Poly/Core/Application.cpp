#include "polypch.h"
#include "Application.h"

Poly::Application::Application()
{
	m_window = new Window(800, 300, "Poly");
	m_window->createConsole();

	Poly::Logger::init();

	POLY_CORE_INFO("Application created!");
}

Poly::Application::~Application()
{
}

void Poly::Application::run()
{
	while (m_running)
	{
		m_window->processMessages();

		for (auto layer : m_layerStack)
			layer->onUpdate();
	}
}

void Poly::Application::pushLayer(Layer* layer)
{
	m_layerStack.pushLayer(layer);
	layer->onAttach();
}

void Poly::Application::pushOverlay(Layer* layer)
{
	m_layerStack.pushOverlay(layer);
	layer->onAttach();
}
