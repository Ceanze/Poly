#pragma once

namespace Poly
{
	class TextureView;
	class Resource;
	class Sampler;
	class Buffer;

	/**
	 * A ResourceView is a non-owning view in to a resource.
	 * The view can be used to send parts of a resource as a read-only container.
	 */
	class ResourceView
	{
	public:
		ResourceView(const TextureView* pTextureView, const Sampler* pSampler);
		ResourceView(const Resource* pResource, uint64 span, uint64 offset);
		ResourceView(const Buffer* pBuffer, uint64 span, uint64 offset);
		~ResourceView() = default;

		static ResourceView Empty() { return {}; }

		bool HasTextureView() const { return m_pTextureView; }
		bool HasSampler() const { return m_pSampler; }
		bool HasBuffer() const { return m_pBuffer; }

		const TextureView* GetTextureView() const { return m_pTextureView; }
		const Sampler* GetSampler() const { return m_pSampler; }
		const Buffer* GetBuffer() const { return m_pBuffer; }

		uint64 GetSpan() const { return m_Span; }
		uint64 GetOffset() const { return m_Offset; }

	private:
		ResourceView() = default;

		const TextureView*	m_pTextureView	= nullptr;
		const Sampler*		m_pSampler		= nullptr;
		const Buffer*		m_pBuffer		= nullptr;

		uint64 m_Span	= 0;
		uint64 m_Offset	= 0;
	};
}