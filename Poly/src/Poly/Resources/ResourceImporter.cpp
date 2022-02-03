#include "ResourceImporter.h"

#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
// #include <yaml-cpp/yaml.h>
#include <fstream>

#include "IOManager.h"

#define PROJECT_POLYRES_FILE "project.polyres"

namespace Poly
{
	void ResourceImporter::LoadImports()
	{
		if (!IOManager::FileExists(PROJECT_POLYRES_FILE))
			return;

		// YAML::Node projectFile = YAML::LoadFile(PROJECT_POLYRES_FILE);

		// if (projectFile["models"])
		// 	for (auto pair : projectFile["models"]) m_PathToID[pair.first.as<std::string>()] = PolyID(pair.second.as<uint64>());

		// if (projectFile["textures"])
		// 	for (auto pair : projectFile["textures"]) m_PathToID[pair.first.as<std::string>()] = PolyID(pair.second.as<uint64>());

		// if (projectFile["materials"])
		// 	for (auto pair : projectFile["materials"]) m_PathToID[pair.first.as<std::string>()] = PolyID(pair.second.as<uint64>());
	}

	PolyID ResourceImporter::GetPathID(const std::string& path)
	{
		if (IsImported(path))
			return m_PathToID.at(path);
		return PolyID::None();
	}

	bool ResourceImporter::IsImported(const std::string& path)
	{
		return m_PathToID.contains(path);
	}

	PolyID ResourceImporter::Import(const std::string& path, ResourceType type)
	{
		if (IsImported(path))
			return m_PathToID[path];

		PolyID pathID = PolyID();
		UpdateProjectFile(path, pathID, type);
		m_PathToID[path] = pathID;
		return pathID;
	}

	PolyID ResourceImporter::ImportModel(const std::string& path)
	{
		return Import(path, ResourceType::MODEL);
	}

	PolyID ResourceImporter::ImportTexture(const std::string& path)
	{
		return Import(path, ResourceType::TEXTURE);
	}

	PolyID ResourceImporter::ImportMaterial(const std::string& path)
	{
		return Import(path, ResourceType::MATERIAL);
	}

	void ResourceImporter::UpdateProjectFile(const std::string& path, PolyID pathID, ResourceType type)
	{
		if (!IOManager::FileExists(PROJECT_POLYRES_FILE))
			CreateProjectFile();

		// YAML::Node projectFile = YAML::LoadFile(PROJECT_POLYRES_FILE);

		// YAML::Node node;
		// node[path] = pathID;

		// switch (type)
		// {
		// 	case ResourceType::MODEL: projectFile["models"].push_back(node); break;
		// 	case ResourceType::TEXTURE: projectFile["textures"].push_back(node); break;
		// 	case ResourceType::MATERIAL: projectFile["material"].push_back(node); break;
		// }

		// std::ofstream file(PROJECT_POLYRES_FILE);
		// file << projectFile;
		// file.close();
	}

	void ResourceImporter::CreateProjectFile()
	{
		// YAML::Emitter out;
		// out << YAML::BeginMap << YAML::Key << "models" << YAML::Value << YAML::BeginSeq << YAML::EndSeq << YAML::EndMap;
		// out << YAML::BeginMap << YAML::Key << "textures" << YAML::Value << YAML::BeginSeq << YAML::EndSeq << YAML::EndMap;
		// out << YAML::BeginMap << YAML::Key << "materials" << YAML::Value << YAML::BeginSeq << YAML::EndSeq << YAML::EndMap;
		// std::ofstream file(PROJECT_POLYRES_FILE);
		// file << out.c_str();
		// file.close();
	}

	std::unordered_map<std::string, PolyID> ResourceImporter::m_PathToID;
}