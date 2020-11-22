#pragma once

namespace Poly
{
	class Application;

	class Engine
	{
	public:
		Engine() = default;
		~Engine() = default;

		static void Init();

		static void Run(Application* pApp);

		static void Release();
	};
}