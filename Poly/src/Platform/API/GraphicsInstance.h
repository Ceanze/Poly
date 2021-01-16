#pragma once

#include "Poly/Core/Core.h"

namespace Poly
{
	// Structs
	struct BufferDesc;

	// Classes
	class Buffer;
	class Window;

	class GraphicsInstance
	{
	public:
		CLASS_ABSTRACT(GraphicsInstance);

		virtual void Init(Window* pWindow) = 0;
		virtual void Cleanup() = 0;

		virtual Ref<Buffer> CreateBuffer(const BufferDesc* pDesc) = 0;
	};
}