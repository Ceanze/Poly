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
		PVKTexture();
		~PVKTexture();

		virtual void Init(const TextureDesc* pDesc) override final;
		void InitWithImage(const TextureDesc* pDesc, VkImage image);

		uint32 GetWidth() const { return p_TextureDesc.Width; }
		uint32 GetHeight() const { return p_TextureDesc.Height; }
		VkFormat GetFormatVK() const { return ConvertFormatVK(p_TextureDesc.Format); }
		VkImage GetImageVK() { return m_Image; }
		VkImageView GetImageViewVK() { return m_ImageView; }

		virtual uint64 GetNative() const override final;

	private:
		void CreateImage();
		void CreateImageView();

		VkImage			m_Image			= VK_NULL_HANDLE;
		VkImageView		m_ImageView		= VK_NULL_HANDLE;
		VmaAllocation	m_Allocation	= VK_NULL_HANDLE;
	};
}