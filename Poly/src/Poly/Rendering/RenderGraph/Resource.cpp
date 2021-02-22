#include "polypch.h"
#include "Resource.h"

#include "Platform/API/Buffer.h"
#include "Platform/API/Texture.h"
#include "Poly/Core/RenderAPI.h"

namespace Poly
{
	Resource::Resource(Ref<Buffer> pBuffer)
	{
		m_pBuffer = pBuffer;
	}

	Resource::Resource(Ref<Texture> pTexture)
	{
		m_pTexture = pTexture;
	}

	Ref<Resource> Resource::Create(Ref<Buffer> pBuffer)
	{
		return CreateRef<Resource>(pBuffer);
	}

	Ref<Resource> Resource::Create(Ref<Texture> pTexture)
	{
		return CreateRef<Resource>(pTexture);
	}
}