#pragma once

#include "Poly/Core/Core.h"

namespace Poly
{
	// Structs
	struct BufferDesc;

	// Classes
	class Buffer;

	class GraphicsInstance
	{
	public:
		CLASS_ABSTRACT(GraphicsInstance);

		virtual Ref<Buffer> CreateBuffer(const BufferDesc* pDesc) = 0;
	};
}