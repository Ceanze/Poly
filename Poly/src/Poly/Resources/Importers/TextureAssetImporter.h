#pragma once

#include "Poly/Resources/AssetTypes/TextureAsset.h"
#include "Poly/Resources/Importers/IAssetImporter.h"

namespace Poly
{
	class TextureAssetImporter : public IAssetImporter
	{
	public:
		// IAssetImporter
		std::vector<std::string> GetSupportedExtensions() const;
		bool                     Import(std::string_view vfsPath, AssetRegistry& registry);

	private:
	};
} // namespace Poly