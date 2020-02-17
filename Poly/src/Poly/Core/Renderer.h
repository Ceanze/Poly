#pragma once

namespace Poly
{

	class Renderer
	{
	public:
		enum class BACKEND { GL45, VULKAN, DX11, DX12 };

		static Renderer* create(BACKEND backend);

		Renderer() = default;
		virtual ~Renderer() = default;
		virtual void initialize(unsigned int width = 800, unsigned int height = 600) = 0;
		virtual void setWinTitle(const char* title) = 0;
		//virtual void present() = 0;
		virtual void shutdown() = 0;

		virtual void setClearColor(float r, float g, float b, float a) = 0;
		virtual void clearBuffer(unsigned int) = 0;
		// submit work (to render) to the renderer.
		//virtual void submit(Mesh* mesh) = 0;
		//virtual void frame() = 0;
	};

}