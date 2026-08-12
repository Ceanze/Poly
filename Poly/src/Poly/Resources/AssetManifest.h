#pragma once

#include "Poly/Resources/AssetID.h"

namespace Poly
{
	struct AssetManifestChild
	{
		AssetID     ID;
		std::string Type; // e.g. MeshAsset, MaterialAsset // TODO: Change to AssetType
		std::string Name; // e.g. "mesh_0", used to build the sub-asset's display path
	};

	// TODO: Don't have this static
	// Sidecar (.meta) metadata for an asset. Only exposes what scanning needs today -
	// import settings/hashing/staleness detection will grow onto this later.
	class AssetManifest
	{
	public:
		/**
		 * Loads just the Children[] table from a .meta sidecar file, without touching the source
		 * asset. Used by AssetRegistry::ScanAssets to register sub-asset IDs up front, since those
		 * IDs only exist once an asset has been imported and are otherwise unknowable from the
		 * source file alone.
		 * @param metaVfsPath VFS path to the .meta sidecar file.
		 */
		static std::vector<AssetManifestChild> LoadChildren(std::string_view metaVfsPath);
	};
} // namespace Poly
