#pragma once

namespace Poly
{
	class Buffer;
	class CommandBuffer;

	class StagingBufferCache
	{
	public:
		StagingBufferCache() = default;
		~StagingBufferCache() = default;

		static Ref<StagingBufferCache> Create() { return CreateRef<StagingBufferCache>(); };

		/**
		 * Queue a staging buffer to transfer data to a GPU buffer
		 * 
		 * @param pStagingBuffer - staging buffer gotten from GetStagingBuffer
		 */
		void QueueTransfer(const Buffer* pDstBuffer, uint64 size, const void* data);

		/**
		 * Submit the transfer of the queued buffers - will submit all at once
		 * which might be unoptimized depending on when they are used
		 * 
		 * @param pCommandBuffer - CommandBuffer to record the transfer on
		 */
		void SubmitQueuedBuffers(CommandBuffer* pCommandBuffer);

		/**
		 * Update the cache to handle deletion of no longer used buffers
		 * 
		 * @param imageIndex
		 */
		void Update(uint32 imageIndex);

	private:
		enum class State
		{
			NONE	= 0,
			IN_USE	= 1,
			FREE	= 2,
		};

		struct BufferPair
		{
			Ref<Buffer> pStagingBuffer;
			const Buffer* pDstBuffer;
		};

		struct LifetimeBuffer
		{
			Ref<Buffer>	pBuffer;
			uint32		Age;
		};

		Ref<Buffer> GetStagingBuffer(uint64 size);

		std::unordered_map<uint32, std::unordered_map<State, std::vector<LifetimeBuffer>>> m_Buffers;
		std::vector<BufferPair> m_QueuedBuffers;
		uint32 m_ImageIndex = 0;
		const uint32 BUFFER_LIFETIME = 100;
	};
}