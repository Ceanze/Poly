#pragma once

#include "PVKTypes.h"

namespace Poly
{

	class PVKImageView
	{
	public:
		PVKImageView();
		~PVKImageView() = default;

		void Init(VkImage image, ImageViewType type, ColorFormat format, ImageAspect aspectMask, uint32_t layerCount = 1);
		void Cleanup();

		VkImageView GetNative() const { return m_ImageView; };

	private:
		VkImageView m_ImageView = VK_NULL_HANDLE;
	};

}