#pragma once

#include "Poly/Rendering/Texture.h"
#include "PVKTypes.h"
#include "polypch.h"
#include "PVKImage.h"
#include "PVKImageView.h"

#include "VmaInclude.h"

namespace Poly
{
	// Old non-portable implementation (to be deleted)
	class PVKTexture
	{
	public:
		PVKTexture();
		~PVKTexture() = default;

		void Init(uint32_t width, uint32_t height, ColorFormat format, ImageUsage usage, ImageCreate flags, uint32_t arrayLayers, uint32_t queueIndices, VmaMemoryUsage memoryUsage);
		void Init(uint32_t width, uint32_t height, ColorFormat format, ImageUsage usage, ImageCreate flags, uint32_t arrayLayers, const std::vector<uint32_t>& queueIndices, VmaMemoryUsage memoryUsage);
		void InitView(ImageViewType type, ImageAspect aspect);
		void Cleanup();

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
		ColorFormat GetFormat() const { return m_Format; }
		PVKImage& GetImage() { return m_Image; }
		PVKImageView& GetImageView() { return m_ImageView; }
		VkMemoryRequirements GetMemoryRequirements() const;

	private:
		uint32_t		m_Width		= 0;
		uint32_t		m_Height	= 0;
		PVKImage		m_Image;
		PVKImageView	m_ImageView;
		ColorFormat		m_Format	= ColorFormat::UNDEFINED;
	};

	class PVKTexture2D : public Texture2D
	{
	public:
		PVKTexture2D(uint32_t width, uint32_t height);
		PVKTexture2D(const std::string& path);
		virtual ~PVKTexture2D();

		virtual void SetData(void* data, uint32_t size) override;

		virtual uint32_t GetHeight() const override { return m_Height; };
		virtual uint32_t GetWidth() const override { return m_Width;	};

	private:
		uint32_t		m_Width		= 0;
		uint32_t		m_Height	= 0;
		std::string 	m_Path		= "";
		PVKImage m_Image;
		PVKImageView m_ImageView;
	};

}