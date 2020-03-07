#pragma once

#include "Renderer.h"

namespace Poly
{
	class RendererAPI
	{
	public:
		enum class BACKEND { GL45, VULKAN, DX11, DX12 };

		static void create(BACKEND backend);
		static void finalize();
		static void initialize(unsigned int width = 800, unsigned int height = 600);
		static void render();
		static void setWinTitle(const char* title);
		static void setClearColor(float r, float g, float b, float a);
		static void shutdown();

	private:
		static Renderer* renderer;
	};

}