#pragma once

namespace Poly
{
	class AssetRegistry;
	class AssetID;

	class IAssetImporter
	{
	public:
		virtual ~IAssetImporter() = default;

		virtual std::vector<std::string> GetSupportedExtensions() const = 0;

		virtual bool Import(std::string_view vfsPath, AssetRegistry& registry) = 0;
	};
} // namespace Poly