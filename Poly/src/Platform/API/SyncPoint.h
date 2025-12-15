#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class SyncPoint;

	struct SyncPointValue
	{
		SyncPoint* pSyncPoint;
		uint64		Value;
	};

	class SyncPoint
	{
	public:
		CLASS_ABSTRACT(SyncPoint);

		/**
		* Init the SyncPoint object
		*/
		virtual void Init() = 0;

		/**
		* Wait for the GPU to signal the provided value to the sync point. Blocks host while doing so
		* @param value - the value to wait for
		*/
		virtual void Wait(uint64 value) = 0;

		/**
		* Signal the sync point with the provided value.
		* @param value - the value to signal
		*/
		virtual void Signal(uint64 value) = 0;

		/**
		* Get the current value of the sync point
		*/
		virtual uint64 GetValue() const = 0;

		/**
		* Add where the sync point should wait or signal, depending what it is submitted as.
		* @param stage - the pipeline stage to wait for. Pipeline stage needs to be available on the current queue for proper use
		*/
		virtual void AddWaitStageMask(FPipelineStage stage) = 0;

		/**
		* Get the current wait mask
		* @return FPipelineStage of the current wait mask
		*/
		virtual FPipelineStage GetWaitStageMask() const = 0;

		/**
		* Clear the current wait mask
		*/
		virtual void ClearWaitStageMask() = 0;

		/**
		* @return Native handle to the API specific object
		*/
		virtual uint64 GetNative() const = 0;
	};
}