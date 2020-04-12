#pragma once

/*
	Interface for sub renderers that is used by the plaform renderer.
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
		virtual void setWindow(Window* window) = 0;
		virtual void setActiveCamera(Camera* camera) = 0;
		virtual void init(IPlatformRenderer* renderer) = 0;
		virtual void beginScene(uint32_t imageIndex) = 0;
		virtual void record() = 0;
		virtual void endScene() = 0;
		virtual void shutdown() = 0;
	};

}