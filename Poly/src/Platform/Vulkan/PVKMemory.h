#pragma once

#include "polypch.h"
#include "PVKTypes.h"

#include <vulkan/vulkan.h>

namespace Poly
{
	class PVKBuffer;
	class PVKTexture;

	class PVKMemory
	{
	public:
		PVKMemory();
		~PVKMemory();

		void init(MemoryPropery memProp);
		void cleanup();

		void bindBuffer(PVKBuffer& buffer);
		void bindTexture(PVKTexture& texture);
		void directTransfer(PVKBuffer& buffer, const void* data, uint64_t size, uint64_t bufferOffset);

		VkDeviceMemory getNative() { return this->memory; }

	private:
		VkDeviceMemory memory;
		std::unordered_map<PVKBuffer*, uint64_t> bufferOffsets;
		std::unordered_map<PVKTexture*, uint64_t> textureOffsets;
		uint64_t currentOffset;
	};
}