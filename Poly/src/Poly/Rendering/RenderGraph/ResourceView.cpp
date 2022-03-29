#include "ResourceView.h"

#include "Resource.h"

namespace Poly
{
	ResourceView::ResourceView(const TextureView* pTextureView, const Sampler* pSampler)
		: m_pTextureView(pTextureView), m_pSampler(pSampler) {}

	ResourceView::ResourceView(const Resource* pResource, uint64 span, uint64 offset)
		: m_pTextureView(pResource->GetAsTextureView()), m_pSampler(pResource->GetAsSampler()), m_pBuffer(pResource->GetAsBuffer())
	{
		m_Span = span;
		m_Offset = offset;
	}

	ResourceView::ResourceView(const Buffer* pBuffer, uint64 span, uint64 offset)
		: m_pBuffer(pBuffer)
	{
		m_Span = span;
		m_Offset = offset;
	}
}