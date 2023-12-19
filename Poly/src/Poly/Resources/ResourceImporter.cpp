#include "ResourceImporter.h"

#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <yaml-cpp/yaml.h>
#include <fstream>

#include "IOManager.h"
#include "Poly/Poly/Format.h"

#define PROJECT_POLYRES_FILE "project.polyres"

namespace Poly
{
	void ResourceImporter::LoadImports()
	{
		if (!IOManager::FileExists(GetProjectPath()))
			return;

		YAML::Node projectFile = YAML::LoadFile(GetProjectPath());

		if (projectFile["models"])
			for (auto pair : projectFile["models"]) m_PathToImportedResource[pair.first.as<std::string>()] = { PolyID(pair.second.as<uint64>()), ResourceType::MODEL };

		if (projectFile["textures"])
			for (auto pair : projectFile["textures"]) m_PathToImportedResource[pair.first.as<std::string>()] = { PolyID(pair.second.as<uint64>()), ResourceType::TEXTURE };

		if (projectFile["materials"])
			for (auto pair : projectFile["materials"]) m_PathToImportedResource[pair.first.as<std::string>()] = { PolyID(pair.second.as<uint64>()), ResourceType::MATERIAL };
	}

	const std::unordered_map<std::string, ResourceImporter::ImportedResource>& ResourceImporter::GetImports()
	{
		return m_PathToImportedResource;
	}

	PolyID ResourceImporter::GetPathID(const std::string& path)
	{
		if (IsImported(path))
			return m_PathToImportedResource.at(path).ResourceID;
		return PolyID::None();
	}

	bool ResourceImporter::IsImported(const std::string& path)
	{
		return m_PathToImportedResource.contains(path);
	}

	PolyID ResourceImporter::Import(const std::string& path, ResourceType type)
	{
		if (IsImported(path))
			return m_PathToImportedResource[path].ResourceID;

		PolyID pathID = PolyID();
		UpdateProjectFile(path, pathID, type);
		m_PathToImportedResource[path] = { pathID, type };
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

	std::string ResourceImporter::GetProjectPath()
	{
		return IOManager::GetRootFolder() + PROJECT_POLYRES_FILE;
	}

	void ResourceImporter::UpdateProjectFile(const std::string& path, PolyID pathID, ResourceType type)
	{
		if (!IOManager::FileExists(GetProjectPath()))
			CreateProjectFile();

		YAML::Node projectFile = YAML::LoadFile(GetProjectPath());

		switch (type)
		{
			case ResourceType::MODEL: projectFile["models"][path] = Poly::Format("{}", static_cast<uint64>(pathID)); break;
			case ResourceType::TEXTURE: projectFile["textures"][path] = Poly::Format("{}", static_cast<uint64>(pathID)); break;
			case ResourceType::MATERIAL: projectFile["material"][path] = Poly::Format("{}", static_cast<uint64>(pathID)); break;
		}

		std::ofstream file(GetProjectPath());
		file << projectFile;
		file.close();
	}

	void ResourceImporter::CreateProjectFile()
	{
		YAML::Emitter out;
		out << YAML::BeginMap << YAML::Key << "models" << YAML::Value << YAML::BeginMap << YAML::EndMap << YAML::EndMap;
		out << YAML::BeginMap << YAML::Key << "textures" << YAML::Value << YAML::BeginMap << YAML::EndMap << YAML::EndMap;
		out << YAML::BeginMap << YAML::Key << "materials" << YAML::Value << YAML::BeginMap << YAML::EndMap << YAML::EndMap;
		std::ofstream file(GetProjectPath());
		file << out.c_str();
		file.close();
	}

	std::unordered_map<std::string, ResourceImporter::ImportedResource> ResourceImporter::m_PathToImportedResource;
}
