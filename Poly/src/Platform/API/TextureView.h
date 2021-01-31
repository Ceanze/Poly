#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class Texture;

	struct TextureViewDesc
	{
		Texture*		pTexture		= nullptr;
		EImageViewType	ImageViewType	= EImageViewType::NONE;
		EFormat			Format			= EFormat::UNDEFINED;
		FImageViewFlag	ImageViewFlag	= FImageViewFlag::NONE;
		uint32			MipLevel		= 0;
		uint32			MipLevelCount	= 0;
		uint32			ArrayLayer		= 0;
		uint32			ArrayLayerCount	= 0;
	};

	class TextureView
	{
	public:
		CLASS_ABSTRACT(TextureView);

		/**
		 * Init the Buffer object
		 * @param desc	Buffer creation description
		*/
		virtual void Init(const TextureViewDesc* pDesc) = 0;

		virtual Texture* GetTexture() const = 0;

		/**
		 * @return Native handle to the API specific object
		 */
		virtual uint64 GetNative() const = 0;

		/**
		 * @return const TextureDesc
		 */
		inline const TextureViewDesc& GetDesc() const
		{
			return p_TextureViewDesc;
		}

	protected:
		TextureViewDesc p_TextureViewDesc;
	};
}