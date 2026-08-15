#pragma once

#include "Poly/Core/Handle.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Poly/Resources/AssetTypes/IAsset.h"

namespace Poly
{
	using TextureHandle = Handle<struct TextureHandleTag>;

	class TextureAsset : public IAsset
	{
	public:
		explicit TextureAsset(AssetID id);

		uint32  GetWidth() const;
		uint32  GetHeight() const;
		EFormat GetFormat() const;

		void SetTextureHandle(TextureHandle textureHandle);

	private:
		TextureHandle m_TextureHandle;
	};
} // namespace Poly