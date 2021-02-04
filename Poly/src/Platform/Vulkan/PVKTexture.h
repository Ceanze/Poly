#pragma once
#include "polypch.h"

#include "Platform/API/Texture.h"
#include "PVKTypes.h"

#include "VmaInclude.h"

namespace Poly
{
	class PVKTexture : public Texture
	{
	public:
		PVKTexture() = default;
		~PVKTexture();

		virtual void Init(const TextureDesc* pDesc) override final;

		/**
		 * Init the texture with only an imageView with the provided image.
		 * The caller of the function is responsable for the life-time of the image.
		 * @param pDesc	- Create description
		 * @param image	- Pre-initilized image by the caller
		 */
		void InitWithImage(const TextureDesc* pDesc, VkImage image);

		uint32 GetWidth() const { return p_TextureDesc.Width; }
		uint32 GetHeight() const { return p_TextureDesc.Height; }
		VkFormat GetFormatVK() const { return ConvertFormatVK(p_TextureDesc.Format); }

		VkImage GetNativeVK() { return m_Image; }
		virtual uint64 GetNative() const override final { return reinterpret_cast<uint64>(m_Image); }

	private:
		void CreateImage();

		VkImage			m_Image			= VK_NULL_HANDLE;
		VmaAllocation	m_Allocation	= VK_NULL_HANDLE;
		bool			m_HandleImage	= false;
	};
}