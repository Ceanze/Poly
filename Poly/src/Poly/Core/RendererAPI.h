#pragma once

#include "Platform/Common/IPlatformRenderer.h"
#include <stdint.h>

namespace Poly
{
	enum class BackendAPI { GL45, VULKAN, DX11, DX12, NONE };

	class RendererAPI
	{
	public:

		static void Create(BackendAPI backend);
		static void CreateRenderer(Renderer subRenderer);
		static void SetActiveCamera(Camera* pCamera);
		static void Init(uint32_t width, uint32_t height);
		static void BeginScene();
		//static void draw(/*Obj*/);
		static void EndScene();
		static void SetWinTitle(const char* title);
		static void SetClearColor(float r, float g, float b, float a);
		static void Shutdown();

		static BackendAPI GetAPI();

		// Create functions here

	private:
		static IPlatformRenderer* s_pRenderer;
		static BackendAPI s_API;
	};

}