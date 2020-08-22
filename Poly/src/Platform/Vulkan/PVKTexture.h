#pragma once

#include "Poly/Renderer/Texture.h"
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

		void init(uint32_t width, uint32_t height, ColorFormat format, ImageUsage usage, ImageCreate flags, uint32_t arrayLayers, uint32_t queueIndices, VmaMemoryUsage memoryUsage);
		void init(uint32_t width, uint32_t height, ColorFormat format, ImageUsage usage, ImageCreate flags, uint32_t arrayLayers, const std::vector<uint32_t>& queueIndices, VmaMemoryUsage memoryUsage);
		void initView(ImageViewType type, ImageAspect aspect);
		void cleanup();

		uint32_t getWidth() const { return this->width; }
		uint32_t getHeight() const { return this->height; }
		ColorFormat getFormat() const { return this->format; }
		PVKImage& getImage() { return this->image; }
		PVKImageView& getImageView() { return this->imageView; }
		VkMemoryRequirements getMemoryRequirements() const;

	private:
		uint32_t width, height;
		PVKImage image;
		PVKImageView imageView;
		ColorFormat format;
	};

	class PVKTexture2D : public Texture2D
	{
	public:
		PVKTexture2D(uint32_t width, uint32_t height);
		PVKTexture2D(const std::string& path);
		virtual ~PVKTexture2D();

		virtual void setData(void* data, uint32_t size) override;

		virtual uint32_t getHeight() const override { return this->height; };
		virtual uint32_t getWidth() const override { return this->width;	};

	private:
		uint32_t width, height;
		std::string path;
		PVKImage image;
		PVKImageView imageView;
	};

}