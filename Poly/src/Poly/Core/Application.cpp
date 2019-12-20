#include "polypch.h"
#include "Application.h"
#include "Poly/Core/Window.h"

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
	}
}
