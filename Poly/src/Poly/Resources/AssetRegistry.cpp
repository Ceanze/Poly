#include "Poly/Resources/AssetRegistry.h"

#include "Poly/Resources/AssetManifest.h"
#include "Poly/Resources/PathUtils.h"
#include "Poly/Resources/VFS/VirtualFileSystem.h"

namespace Poly
{
	void AssetRegistry::ScanAssets()
	{
		// "/" finds all files on all mounted backends
		for (const std::string& path : VirtualFileSystem::EnumerateFiles("/"))
		{
			if (PathUtils::GetExtension(path) == "meta")
			{
				std::string sourcePath = path.substr(0, path.size() - std::string_view(".meta").size());
				if (VirtualFileSystem::Exists(sourcePath))
				{
					for (const AssetManifestChild& child : AssetManifest::LoadChildren(path))
						m_IDToPath[child.ID] = sourcePath + "#" + child.Name;
				}

				continue;
			}

			m_IDToPath[AssetID(path)] = path;
		}
	}

	const std::string& AssetRegistry::ResolvePath(AssetID assetID) const
	{
		const auto itr = m_IDToPath.find(assetID);
		if (itr == m_IDToPath.end())
		{
			POLY_CORE_WARN("Asset ID {} could not be found in registry", assetID);
			static const std::string s_EmptyPath;
			return s_EmptyPath;
		}

		return itr->second;
	}

	void AssetRegistry::RegisterPath(AssetID assetID, std::string_view vfsPath)
	{
		m_IDToPath.try_emplace(assetID, vfsPath);
	}
} // namespace Poly