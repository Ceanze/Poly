#include "StagingBufferCache.h"

#include "Platform/API/Buffer.h"
#include "Platform/API/CommandBuffer.h"

#include "Poly/Core/RenderAPI.h"

namespace Poly
{
	void StagingBufferCache::QueueTransfer(const Buffer* pDstBuffer, uint64 size, uint64 offset, const void* data)
	{
		Ref<Buffer> pStagingBuffer = nullptr;
		bool alreadyQueued = false;
		if (m_BufferToIndex.contains(pDstBuffer))
		{
			pStagingBuffer = m_QueuedBuffers[m_BufferToIndex[pDstBuffer]].pStagingBuffer;
			alreadyQueued = true;
		}
		else
		{
			uint64 maxSize = pDstBuffer->GetSize();
			pStagingBuffer = GetStagingBuffer(maxSize);
		}

		pStagingBuffer->TransferData(data, size, offset);

		BufferRegion region = { .Size = size, .SrcOffset = offset, .DstOffset = offset };

		if (alreadyQueued)
		{
			m_QueuedBuffers[m_BufferToIndex[pDstBuffer]].Regions.push_back(region);
		}
		else
		{
			m_BufferToIndex[pDstBuffer] = m_QueuedBuffers.size();
			m_QueuedBuffers.push_back({ pStagingBuffer, pDstBuffer, {region} });
		}
	}

	void StagingBufferCache::SubmitQueuedBuffers(CommandBuffer* pCommandBuffer)
	{
		for (auto& bufferPair : m_QueuedBuffers)
		{
			if (bufferPair.Regions.size() == 1)
			{
				const BufferRegion& region = bufferPair.Regions.front();
				pCommandBuffer->CopyBuffer(bufferPair.pStagingBuffer.get(), bufferPair.pDstBuffer, region.Size, region.SrcOffset, region.DstOffset);
			}
			else
			{
				pCommandBuffer->CopyBufferRegions(bufferPair.pStagingBuffer.get(), bufferPair.pDstBuffer, bufferPair.Regions);
			}
		}

		m_QueuedBuffers.clear();
		m_BufferToIndex.clear();
	}

	void StagingBufferCache::Update(uint32 imageIndex)
	{
		m_ImageIndex = imageIndex;

		auto& buffersInUse = m_Buffers[m_ImageIndex][State::IN_USE];
		auto& buffersFree = m_Buffers[m_ImageIndex][State::FREE];

		// Delete old & update age of young buffers
		auto it = buffersFree.begin();
		while (it != buffersFree.end())
		{
			if (it->Age > BUFFER_LIFETIME)
				it = buffersFree.erase(it);
			else
			{
				it->Age++;
				it++;
			}
		}

		// Mark IN_USE buffers FREE
		buffersFree.insert(buffersFree.end(), buffersInUse.begin(), buffersInUse.end());
		buffersInUse.clear();
	}

	Ref<Buffer> StagingBufferCache::GetStagingBuffer(uint64 size)
	{
		auto& buffers = m_Buffers[m_ImageIndex][State::FREE];

		// Find fitting buffer
		auto bufferItr = std::find_if(buffers.begin(), buffers.end(), [size](const LifetimeBuffer& lb) { return size <= lb.pBuffer->GetSize(); });

		// No free buffer which suits the requirements
		if (bufferItr == buffers.end())
		{
			BufferDesc desc = {};
			desc.BufferUsage	= FBufferUsage::COPY_SRC;
			desc.MemUsage		= EMemoryUsage::CPU_VISIBLE;
			desc.Size			= size;
			Ref<Buffer> pBuffer = RenderAPI::CreateBuffer(&desc);

			// It is assumed that the staging buffer gotten will be used that frame
			m_Buffers[m_ImageIndex][State::IN_USE].push_back({ pBuffer, 0});
			return pBuffer;
		}

		Ref<Buffer> pBuffer = (*bufferItr).pBuffer;
		buffers.erase(bufferItr);
		m_Buffers[m_ImageIndex][State::IN_USE].push_back({ pBuffer, 0 });
		return pBuffer;
	}
}