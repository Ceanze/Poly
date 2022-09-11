#pragma once

#include "ResourceTypes.h"

class aiNode;
class aiScene;

namespace YAML
{
	class Emitter;
	class Node;
}

namespace Poly
{
	class ResourceImporter
	{
	public:
		struct ImportedResource
		{
			PolyID ResourceID;
			ResourceType Type;
		};

	public:
		ResourceImporter() = default;
		~ResourceImporter() = default;

		static void LoadImports();

		static const std::unordered_map<std::string, ImportedResource>& GetImports();

		static PolyID GetPathID(const std::string& path);

		static bool IsImported(const std::string& path);

		static PolyID Import(const std::string& path, ResourceType type);

		static PolyID ImportModel(const std::string& path);

		static PolyID ImportTexture(const std::string& path);

		static PolyID ImportMaterial(const std::string& path);

	private:
		static void UpdateProjectFile(const std::string& path, PolyID pathID, ResourceType type);

		static void CreateProjectFile();

		static std::unordered_map<std::string, ImportedResource> m_PathToImportedResource;
	};
}