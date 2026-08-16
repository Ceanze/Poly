#include "TextureAsset.h"

#include "Poly/RenderGraph/ResourceManager.h"

namespace Poly
{
	TextureAsset::TextureAsset(AssetID id)
	{
		p_ID = id;
	}

	uint32 TextureAsset::GetWidth() const
	{
		Texture* pTexture = ResourceManager::Resolve(m_TextureHandle);
		if (!pTexture)
			return 0;

		return pTexture->GetWidth();
	}

	uint32 TextureAsset::GetHeight() const
	{
		Texture* pTexture = ResourceManager::Resolve(m_TextureHandle);
		if (!pTexture)
			return 0;

		return pTexture->GetHeight();
	}

	EFormat TextureAsset::GetFormat() const
	{
		TextureView* pTextureView = ResourceManager::ResolveView(m_TextureHandle);
		if (!pTextureView)
			return EFormat::UNDEFINED;

		return pTextureView->GetDesc().Format;
	}

	TextureHandle TextureAsset::GetHandle() const
	{
		return m_TextureHandle;
	}

	void TextureAsset::SetTextureHandle(TextureHandle textureHandle)
	{
		m_TextureHandle = textureHandle;
	}
} // namespace Poly