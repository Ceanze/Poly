#pragma once

#include "Poly/RenderGraph/BufferRange.h"
#include "Poly/RenderGraph/ResourceManager.h"

#include <string>

namespace Poly
{
	/*
	 * Append-only (bump) allocator over a single GPU buffer that grows on demand. Never frees
	 * individual allocations - intended for data that's loaded once and kept alive for the
	 * process's lifetime (e.g. mesh geometry, via GeometryPool). Not thread-safe: callers must
	 * synchronize externally if Upload() can be called from more than one thread.
	 */
	class BufferArena
	{
	public:
		BufferArena(uint64 elementStride, FBufferUsage usage, EMemoryUsage memUsage, std::string debugName);
		~BufferArena() = default;
		CLASS_REMOVE_COPY(BufferArena);

		/*
		 * Allocates `count` elements and uploads pData into them, growing the underlying buffer
		 * first if it doesn't have room.
		 * @param pData - pointer to `count` elements worth of data (each ElementStride bytes)
		 * @param count - number of elements to allocate and upload
		 * @return BufferRange - the element range the data was placed at
		 */
		BufferRange Upload(const void* pData, uint32 count);

		// Drops the arena back to its unallocated state without touching the underlying buffer -
		// for use during engine shutdown, after ResourceManager has already torn everything down.
		void Reset();

		BufferHandle GetBufferHandle() const { return m_Handle; }
		uint32       GetCapacity() const { return m_ElementCapacity; }
		uint32       GetCount() const { return m_ElementCount; }

	private:
		void EnsureCapacity(uint32 requiredCount);

		uint64       m_ElementStride;
		FBufferUsage m_Usage;
		EMemoryUsage m_MemUsage;
		std::string  m_DebugName;

		BufferHandle m_Handle;
		uint32       m_ElementCapacity = 0;
		uint32       m_ElementCount    = 0;
	};
} // namespace Poly
