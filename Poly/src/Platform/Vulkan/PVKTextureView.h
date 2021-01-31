#pragma once

#include "Platform/API/TextureView.h"
#include "PVKTypes.h"

namespace Poly
{
	class PVKTextureView : public TextureView
	{
	public:
		PVKTextureView() = default;
		~PVKTextureView();

		virtual void Init(const TextureViewDesc* pDesc) override final;

		virtual Texture* GetTexture() const override final { return p_TextureViewDesc.pTexture; };
		virtual uint64 GetNative() const override final { return reinterpret_cast<uint64>(m_ImageView); }
		VkImageView GetNativeVK() const { return m_ImageView; }

	private:
		VkImageView m_ImageView = VK_NULL_HANDLE;
	};
}