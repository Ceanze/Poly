#pragma once

/*
	Interface for sub renderers that is used by the plaform renderer.

	Camera is available for all sub-renderers
*/

#include <stdint.h>

namespace Poly
{

	class Camera;
	class Window;
	class IPlatformRenderer;

	class IRenderer
	{
	public:
		IRenderer() = default;
		virtual ~IRenderer() = default;
		virtual void SetWindow(Window* window) = 0;
		virtual void SetActiveCamera(Camera* camera) { m_pCamera = camera; }
		virtual void Init(IPlatformRenderer* renderer) = 0;
		virtual void BeginScene(uint32_t imageIndex) = 0;
		virtual void Record() = 0;
		virtual void EndScene() = 0;

	protected:
		Camera* m_pCamera;
	};

}