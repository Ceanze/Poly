#pragma once

#include "Poly/Core/Core.h"

namespace Poly
{
	struct BufferDesc
	{

	};

	class Buffer
	{
	public:
		CLASS_ABSTRACT(Buffer);

		/**
		 * Init the Buffer object
		 * @param desc	Buffer creation description
		*/
		virtual void Init(const BufferDesc* pDesc) = 0;

		/**
		 * Map to CPU memory for transfer of data
		 * @return Pointer to the now mapped memory
		 */
		virtual void* Map() = 0;

		/**
		 * Transfer memory directly to buffer, shorthand for
		 * Map(), memcpy, Unmap() process.
		 * @param data	pointer to data being copied over
		 * @param size	size in bytes of data being copied
		 */
		virtual void TransferData(const void* data, const size_t size) = 0;

		/**
		 * Unmap buffer. Previous mapped pointer becomes invalid
		 */
		virtual void Unmap() = 0;

		/**
		 * @return Native handle to the API specific object
		 */
		virtual uint64_t GetNative() const = 0;

		/**
		 * @return Alignment for the buffer in memory
		 */
		virtual uint64_t GetAlignment() const = 0;

		/**
		 * @return Size of buffer in memory
		 */
		virtual uint64_t GetSize() const = 0;
	};
}