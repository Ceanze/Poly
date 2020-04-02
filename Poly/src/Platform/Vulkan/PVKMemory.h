#pragma once

#include "polypch.h"
#include "PVKTypes.h"

#include <vulkan/vulkan.h>

namespace Poly
{
	class PVKBuffer;

	class PVKMemory
	{
	public:
		PVKMemory();
		~PVKMemory();

		void init(MemoryPropery memProp);
		void cleanup();

		void bindBuffer(PVKBuffer& buffer);
		//void bindTexture(Texture* texture);
		void directTransfer(PVKBuffer& buffer, const void* data, uint64_t size, uint64_t bufferOffset);

		VkDeviceMemory getMemory();

	private:
		VkDeviceMemory memory;
		std::unordered_map<PVKBuffer*, uint64_t> bufferOffsets;
		//std::unordered_map<Texture*, uint64_t> textureOffsets;
		uint64_t currentOffset;
	};
}