#pragma once

#include "Platform/Common/IPlatformRenderer.h"
#include <stdint.h>

namespace Poly
{
	enum class BackendAPI { GL45, VULKAN, DX11, DX12, NONE };

	class RendererAPI
	{
	public:

		static void create(BackendAPI backend);
		static void createRenderer(Renderer subRenderer);
		static void setActiveCamera(Camera* camera);
		static void init(uint32_t width, uint32_t height);
		static void beginScene();
		//static void draw(/*Obj*/);
		static void endScene();
		static void setWinTitle(const char* title);
		static void setClearColor(float r, float g, float b, float a);
		static void shutdown();

		static BackendAPI getAPI();

		// Create functions here

	private:
		static IPlatformRenderer* renderer;
		static BackendAPI api;
	};

}