#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class BinarySemaphore
	{
	public:
		CLASS_ABSTRACT(BinarySemaphore);

		/**
		 * Init the BinarySemaphore object
		 * @param desc - BinarySemaphore creation description
		*/
		virtual void Init() = 0;

		virtual void AddWaitStageMask(FPipelineStage stage) = 0;
		virtual FPipelineStage GetWaitStageMask() const = 0;
		virtual void ClearWaitStageMask() = 0;

		/**
		 * @return Native handle to the API specific object
		 */
		virtual uint64 GetNative() const = 0;
	};
}