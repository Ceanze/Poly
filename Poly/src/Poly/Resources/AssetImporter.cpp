#include "AssetImporter.h"

#include "Poly/Resources/VFS/VirtualFileSystem.h"

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <yaml-cpp/yaml.h>

#include <fstream>

#define PROJECT_POLYRES_FILE "project.polyres"

namespace Poly
{
	void AssetImporter::LoadImports()
	{
		if (!VirtualFileSystem::Exists(GetProjectPath()))
			return;

		YAML::Node projectFile = YAML::Load(VirtualFileSystem::ReadText(GetProjectPath()));

		if (projectFile["models"])
			for (auto pair : projectFile["models"])
				m_PathToImportedResource[pair.first.as<std::string>()] = {PolyID(pair.second.as<uint64>()), ResourceType::MODEL};

		if (projectFile["textures"])
			for (auto pair : projectFile["textures"])
				m_PathToImportedResource[pair.first.as<std::string>()] = {PolyID(pair.second.as<uint64>()), ResourceType::TEXTURE};

		if (projectFile["materials"])
			for (auto pair : projectFile["materials"])
				m_PathToImportedResource[pair.first.as<std::string>()] = {PolyID(pair.second.as<uint64>()), ResourceType::MATERIAL};
	}

	const std::unordered_map<std::string, AssetImporter::ImportedResource>& AssetImporter::GetImports()
	{
		return m_PathToImportedResource;
	}

	PolyID AssetImporter::GetPathID(const std::string& path)
	{
		if (IsImported(path))
			return m_PathToImportedResource.at(path).ResourceID;
		return PolyID::None();
	}

	bool AssetImporter::IsImported(const std::string& path)
	{
		return m_PathToImportedResource.contains(path);
	}

	PolyID AssetImporter::Import(const std::string& path, ResourceType type)
	{
		if (IsImported(path))
			return m_PathToImportedResource[path].ResourceID;

		PolyID pathID = PolyID();
		UpdateProjectFile(path, pathID, type);
		m_PathToImportedResource[path] = {pathID, type};
		return pathID;
	}

	PolyID AssetImporter::ImportModel(const std::string& path)
	{
		return Import(path, ResourceType::MODEL);
	}

	PolyID AssetImporter::ImportTexture(const std::string& path)
	{
		return Import(path, ResourceType::TEXTURE);
	}

	PolyID AssetImporter::ImportMaterial(const std::string& path)
	{
		return Import(path, ResourceType::MATERIAL);
	}

	std::string AssetImporter::GetProjectPath()
	{
		return "compat/" + std::string(PROJECT_POLYRES_FILE);
	}

	void AssetImporter::UpdateProjectFile(const std::string& path, PolyID pathID, ResourceType type)
	{
		if (!VirtualFileSystem::Exists(GetProjectPath()))
			CreateProjectFile();

		YAML::Node projectFile = YAML::Load(VirtualFileSystem::ReadText(GetProjectPath()));
		uint64     id          = static_cast<uint64>(pathID);

		switch (type)
		{
		case ResourceType::MODEL:
			projectFile["models"][path] = std::to_string(id);
			break;
		case ResourceType::TEXTURE:
			projectFile["textures"][path] = std::to_string(id);
			break;
		case ResourceType::MATERIAL:
			projectFile["material"][path] = std::to_string(id);
			break;
		}

		std::ofstream file(GetProjectPath());
		file << projectFile;
		file.close();
	}

	void AssetImporter::CreateProjectFile()
	{
		YAML::Emitter out;
		out << YAML::BeginMap << YAML::Key << "models" << YAML::Value << YAML::BeginMap << YAML::EndMap << YAML::EndMap;
		out << YAML::BeginMap << YAML::Key << "textures" << YAML::Value << YAML::BeginMap << YAML::EndMap << YAML::EndMap;
		out << YAML::BeginMap << YAML::Key << "materials" << YAML::Value << YAML::BeginMap << YAML::EndMap << YAML::EndMap;
		VirtualFileSystem::WriteText(GetProjectPath(), out.c_str());
	}

	std::unordered_map<std::string, AssetImporter::ImportedResource> AssetImporter::m_PathToImportedResource;
} // namespace Poly
