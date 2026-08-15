#include "TextureAssetImporter.h"

#include "Poly/RenderGraph/ResourceManager.h"
#include "Poly/Resources/AssetRegistry.h"
#include "Poly/Resources/PathUtils.h"
#include "Poly/Resources/VFS/VirtualFileSystem.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Poly
{
	std::vector<std::string> TextureAssetImporter::GetSupportedExtensions() const
	{
		// What STB supports (except .hdr)
		// TODO: HDR isn't fully supported due to not supporting non-4-byte-wide images
		return {".png", ".gif", ".jpeg", ".jpg", ".tga", ".bmp", ".psd", ".pic", ".pnm"};
	}

	bool TextureAssetImporter::Import(std::string_view vfsPath, AssetRegistry& registry)
	{
		std::vector<byte> content = VirtualFileSystem::Read(vfsPath);

		if (content.empty())
		{
			POLY_CORE_WARN("Failed to load {}", vfsPath);
			return false;
		}

		int texWidth  = 0;
		int texHeight = 0;
		int channels  = 0;

		byte* data = stbi_load_from_memory(content.data(), content.size(), &texWidth, &texHeight, &channels, STBI_rgb_alpha);
		if (!data)
		{
			POLY_CORE_ERROR("Failed to load image {}", vfsPath);
			return false;
		}

		TextureHandle textureHandle = ResourceManager::CreateTexture2D(
		    texWidth,
		    texHeight,
		    EFormat::R8G8B8A8_UNORM,
		    FTextureUsage::TRANSFER_DST | FTextureUsage::TRANSFER_SRC | FTextureUsage::SAMPLED,
		    PathUtils::GetFileName(vfsPath));

		ResourceManager::UploadTextureData(textureHandle, data, texWidth, texHeight);

		AssetID      assetID(vfsPath);
		TextureAsset textureAsset(assetID);
		textureAsset.SetTextureHandle(textureHandle);

		registry.Emplace<TextureAsset>(assetID, std::move(textureAsset));

		stbi_image_free(data);

		return true;
	}
} // namespace Poly