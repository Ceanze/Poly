#include "Poly/Resources/AssetManifest.h"

#include "Poly/Resources/VFS/VirtualFileSystem.h"

#include <yaml-cpp/yaml.h>

namespace Poly
{
	std::vector<AssetManifestChild> AssetManifest::LoadChildren(std::string_view metaVfsPath)
	{
		std::vector<AssetManifestChild> children;

		YAML::Node manifest     = YAML::Load(VirtualFileSystem::ReadText(metaVfsPath));
		YAML::Node childrenNode = manifest["Children"];
		if (!childrenNode || !childrenNode.IsSequence())
			return children;

		children.reserve(childrenNode.size());
		for (const YAML::Node& child : childrenNode)
		{
			AssetManifestChild entry;
			entry.ID   = AssetID(child["AssetID"].as<uint64>());
			entry.Type = child["Type"].as<std::string>("");
			entry.Name = child["Name"].as<std::string>("");
			children.push_back(std::move(entry));
		}

		return children;
	}
} // namespace Poly
