#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	struct TextureDesc
	{
		uint32			Width			= 0;
		uint32			Height			= 0;
		uint32			Depth			= 1;
		uint32			ArrayLayers		= 0;
		uint32			MipLevels		= 0;
		uint32			SampleCount		= 0;
		EMemoryUsage	MemoryUsage		= EMemoryUsage::UNKNOWN;
		EFormat			Format			= EFormat::UNDEFINED;
		FTextureUsage	TextureUsage	= FTextureUsage::NONE;
		ETextureDim		TextureDim		= ETextureDim::NONE;
	};

	class Texture
	{
	public:
		CLASS_ABSTRACT(Texture);

		/**
		 * Init the Buffer object
		 * @param desc	Buffer creation description
		*/
		virtual void Init(const TextureDesc* pDesc) = 0;

		/**
		 * @return Native handle to the API specific object
		 */
		virtual uint64 GetNative() const = 0;

		/**
		 * @return const TextureDesc
		 */
		inline const TextureDesc& GetDesc() const
		{
			return p_TextureDesc;
		}

	protected:
		TextureDesc p_TextureDesc;
	};
}