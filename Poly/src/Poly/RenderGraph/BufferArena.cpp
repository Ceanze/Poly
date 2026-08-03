#include "BufferArena.h"

#include <algorithm>

namespace Poly
{
	BufferArena::BufferArena(uint64 elementStride, FBufferUsage usage, EMemoryUsage memUsage, std::string debugName)
	    : m_ElementStride(elementStride)
	    , m_Usage(usage | FBufferUsage::TRANSFER_SRC)
	    , m_MemUsage(memUsage)
	    , m_DebugName(std::move(debugName))
	{}

	BufferRange BufferArena::Upload(const void* pData, uint32 count)
	{
		EnsureCapacity(m_ElementCount + count);

		const BufferRange range{m_ElementCount, count};
		ResourceManager::UploadBufferData(m_Handle, pData, static_cast<uint64>(count) * m_ElementStride,
		                                  static_cast<uint64>(range.ElementOffset) * m_ElementStride);
		m_ElementCount += count;
		return range;
	}

	void BufferArena::Reset()
	{
		m_Handle          = {};
		m_ElementCapacity = 0;
		m_ElementCount    = 0;
	}

	void BufferArena::EnsureCapacity(uint32 requiredCount)
	{
		if (requiredCount <= m_ElementCapacity)
			return;

		constexpr uint32 kMinCapacity = 1024;
		uint32           newCapacity  = m_ElementCapacity == 0 ? kMinCapacity : m_ElementCapacity * 2;
		newCapacity                   = std::max(newCapacity, requiredCount);

		if (!m_Handle.IsValid())
			m_Handle = ResourceManager::CreateBuffer(static_cast<uint64>(newCapacity) * m_ElementStride, m_Usage, m_MemUsage, m_DebugName);
		else
			m_Handle = ResourceManager::ResizeBuffer(m_Handle, static_cast<uint64>(newCapacity) * m_ElementStride);

		m_ElementCapacity = newCapacity;
	}
} // namespace Poly
