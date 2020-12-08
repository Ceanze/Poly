#pragma once

#include "Platform/Common/IPlatformRenderer.h"
#include <stdint.h>

/*
*	TODO: This file was previously created for a higher level of abstraction. Since Poly isn't
*			at that step yet, it will not be necessary for most of the project.
*			A new class "RenderAPI" has instead been created, which will act as a lower
*			level of abstraction to create the necessary API objects.
*		The PlatformRenderer will also be affected by this, as it is also expected to be used
*		by a higher level abstraction.
*
*		The implementation ov having specific draw calls for different objects and the begin and
*		end scene will be put on hold.
*/

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