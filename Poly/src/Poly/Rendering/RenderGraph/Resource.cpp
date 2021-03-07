#include "polypch.h"
#include "Resource.h"

#include "Platform/API/Buffer.h"
#include "Platform/API/Texture.h"
#include "Poly/Core/RenderAPI.h"

namespace Poly
{
	Resource::Resource(Ref<Buffer> pBuffer, const std::string& name)
	{
		m_pBuffer = pBuffer;
		m_Name = name;
	}

	Resource::Resource(Ref<Texture> pTexture, const std::string& name)
	{
		m_pTexture = pTexture;
		m_Name = name;
	}

	Ref<Resource> Resource::Create(Ref<Buffer> pBuffer, const std::string& name)
	{
		return CreateRef<Resource>(pBuffer, name);
	}

	Ref<Resource> Resource::Create(Ref<Texture> pTexture, const std::string& name)
	{
		return CreateRef<Resource>(pTexture, name);
	}
}