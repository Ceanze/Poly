#pragma once

#include "polypch.h"
#include "PVKTypes.h"

namespace Poly
{
	class PVKBuffer;
	class PVKTexture;

	class PVKMemory
	{
	public:
		PVKMemory();
		~PVKMemory();

		void Init(MemoryPropery memProp);
		void Cleanup();

		void BindBuffer(PVKBuffer& buffer);
		void BindTexture(PVKTexture& texture);
		void DirectTransfer(PVKBuffer& buffer, const void* pData, uint64_t size, uint64_t bufferOffset);

		VkDeviceMemory GetNative() { return m_Memory; }

	private:
		VkDeviceMemory m_Memory = VK_NULL_HANDLE;
		std::unordered_map<PVKBuffer*, uint64_t> m_BufferOffsets;
		std::unordered_map<PVKTexture*, uint64_t> m_TextureOffsets;
		uint64_t m_CurrentOffset = 0;
	};
}