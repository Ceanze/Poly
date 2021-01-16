#pragma once

#include "polypch.h"
#include "PVKTypes.h"
#include "Platform/API/Buffer.h"

#include "VmaInclude.h"

namespace Poly
{
	class PVKBuffer : public Buffer
	{
	public:
		PVKBuffer();
		~PVKBuffer();

		virtual void Init(const BufferDesc* pDesc) override final;
		void Cleanup();

		virtual void* Map() override final;
		virtual void TransferData(const void* data, const size_t size) override final;
		virtual void Unmap() override final;

		virtual uint64 GetSize()		const override final;
		virtual uint64 GetAlignment()	const override final;
		virtual uint64 GetNative()		const override final;

		VkBuffer GetNativeVK() const;

	private:
		VkBuffer		m_Buffer			= VK_NULL_HANDLE;
		VmaAllocation	m_VmaAllocation		= VK_NULL_HANDLE;
		bool			m_Mapped			= false;
		void*			m_MappedPtr			= nullptr;
	};
}