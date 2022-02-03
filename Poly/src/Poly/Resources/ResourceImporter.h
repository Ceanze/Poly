#pragma once

class aiNode;
class aiScene;

namespace YAML
{
	class Emitter;
	class Node;
}

namespace Poly
{
	struct MeshMaterialIDPair
	{
		PolyID MeshID;
		PolyID MaterialID;
	};

	class ResourceImporter
	{
	public:
		enum class ResourceType
		{
			NONE		= 0,
			MODEL		= 1,
			TEXTURE		= 2,
			MATERIAL	= 3,
		};

	public:
		ResourceImporter() = default;
		~ResourceImporter() = default;

		static void LoadImports();

		static PolyID GetPathID(const std::string& path);

		static bool IsImported(const std::string& path);

		static PolyID Import(const std::string& path, ResourceType type);

		static PolyID ImportModel(const std::string& path);

		static PolyID ImportTexture(const std::string& path);

		static PolyID ImportMaterial(const std::string& path);

	private:
		static void UpdateProjectFile(const std::string& path, PolyID pathID, ResourceType type);

		static void CreateProjectFile();

		static std::unordered_map<std::string, PolyID> m_PathToID;
	};
}