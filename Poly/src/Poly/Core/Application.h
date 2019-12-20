#pragma once

namespace Poly {

	class Window;

	class Application
	{
	public:
		Application();
		virtual ~Application();

		void run();
	private:
		bool m_running = true;
		Window* m_window;
	};

	Application* CreateApplication();

}