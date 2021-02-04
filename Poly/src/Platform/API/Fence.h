#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class Fence
	{
	public:
		CLASS_ABSTRACT(Fence);

		/**
		 * Init the Buffer object
		 * @param desc	Buffer creation description
		*/
		virtual void Init(FFenceFlag fenceFlag) = 0;

		virtual void Reset() = 0;

		virtual void Wait(uint64 timeout) = 0;

		/**
		 * @return Native handle to the API specific object
		 */
		virtual uint64 GetNative() const = 0;
	};
}