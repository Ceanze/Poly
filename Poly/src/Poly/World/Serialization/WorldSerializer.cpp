#include "WorldSerializer.h"

#include "Poly/Core/Serialization/YamlConversions.h"
#include "Poly/Resources/AssetHandler.h"
#include "Poly/Resources/AssetTypes/MaterialAsset.h"
#include "Poly/Resources/AssetTypes/MeshAsset.h"
#include "Poly/Resources/VFS/VirtualFileSystem.h"
#include "Poly/Scene/Components.h"
#include "Poly/Scene/Components/MaterialComponent.h"
#include "Poly/Scene/Components/MeshAssetComponent.h"
#include "Poly/Scene/Components/NameComponent.h"

#include <yaml-cpp/yaml.h>

#include <unordered_map>
#include <unordered_set>

namespace
{
	template<typename AssetType>
	void SerializeAssetPath(YAML::Emitter& out, const char* key, Poly::AssetHandle<AssetType> handle)
	{
		std::string path = Poly::AssetHandler::GetPath(handle);
		if (path.empty())
			POLY_CORE_WARN("Cannot serialize '{}', asset handle {} has no known path", key, handle.Get());

		out << YAML::Key << key << YAML::Value << path;
	}

	template<typename AssetType>
	bool DeserializeAssetPath(const YAML::Node& node, const char* key, Poly::AssetHandle<AssetType>& handle)
	{
		const std::string path = node[key].as<std::string>("");
		if (path.empty())
			return false;

		handle = Poly::AssetHandler::Load<AssetType>(path);
		return handle.IsValid();
	}
} // namespace

namespace Poly
{
	WorldSerializer::WorldSerializer()
	{
		RegisterBuiltInComponents();
	}

	bool WorldSerializer::Save(const World& world, std::string_view vfsPath) const
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Version" << YAML::Value << FORMAT_VERSION;
		out << YAML::Key << "World" << YAML::Value << world.GetName();
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		auto hierarchies = world.View<IDComponent, HierarchyComponent>();

		// Depth first from the roots, so parents are written before their children and siblings keep their order.
		// The stack is filled in reverse, so the first root/child is popped first
		std::vector<entt::entity> stack;
		for (entt::entity entity : hierarchies)
		{
			if (hierarchies.get<HierarchyComponent>(entity).Parent == entt::null)
				stack.push_back(entity);
		}
		std::reverse(stack.begin(), stack.end());

		std::vector<entt::entity> children;
		while (!stack.empty())
		{
			const entt::entity entity = stack.back();
			stack.pop_back();

			SerializeEntity(out, world, entity);

			// Children form a circular list starting at First
			children.clear();
			const entt::entity first = hierarchies.get<HierarchyComponent>(entity).First;
			for (entt::entity child = first; child != entt::null;)
			{
				children.push_back(child);

				child = hierarchies.get<HierarchyComponent>(child).Next;
				if (child == first)
					break;
			}
			stack.insert(stack.end(), children.rbegin(), children.rend());
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		if (!out.good())
		{
			POLY_CORE_WARN("Cannot save world '{}' to {}, yaml emitter failed: {}", world.GetName(), vfsPath, out.GetLastError());
			return false;
		}

		if (!VirtualFileSystem::WriteText(vfsPath, out.c_str()))
		{
			POLY_CORE_WARN("Cannot save world '{}', failed to write {}", world.GetName(), vfsPath);
			return false;
		}

		return true;
	}

	bool WorldSerializer::Load(World& world, std::string_view vfsPath) const
	{
		if (!world.IsEmpty())
		{
			POLY_CORE_WARN("Cannot load world from {}, world '{}' is not empty", vfsPath, world.GetName());
			return false;
		}

		if (!VirtualFileSystem::Exists(vfsPath))
		{
			POLY_CORE_WARN("Cannot load world from {}, file cannot be found", vfsPath);
			return false;
		}

		struct EntityEntry
		{
			uint64     ID     = 0;
			uint64     Parent = 0;
			YAML::Node Components;
		};

		std::string              worldName;
		std::vector<EntityEntry> entries;

		// Read and validate everything before touching the world, so an invalid file leaves it untouched
		try
		{
			const YAML::Node root = YAML::Load(VirtualFileSystem::ReadText(vfsPath));

			const uint32 version = root["Version"].as<uint32>(0);
			if (version != FORMAT_VERSION)
			{
				POLY_CORE_WARN("Cannot load world from {}, format version {} is not supported (expected {})", vfsPath, version, FORMAT_VERSION);
				return false;
			}

			worldName = root["World"].as<std::string>(world.GetName());

			const YAML::Node entitiesNode = root["Entities"];
			if (entitiesNode && !entitiesNode.IsSequence())
			{
				POLY_CORE_WARN("Cannot load world from {}, 'Entities' is not a sequence", vfsPath);
				return false;
			}

			std::unordered_set<uint64> ids;
			for (const YAML::Node& entityNode : entitiesNode)
			{
				EntityEntry entry;
				entry.ID         = entityNode["ID"].as<uint64>(0);
				entry.Parent     = entityNode["Parent"].as<uint64>(0);
				entry.Components = entityNode["Components"];

				if (entry.ID == 0 || !ids.insert(entry.ID).second)
				{
					POLY_CORE_WARN("Cannot load world from {}, entity ID {} is missing or duplicated", vfsPath, entry.ID);
					return false;
				}

				if (entry.Components && !entry.Components.IsMap())
				{
					POLY_CORE_WARN("Cannot load world from {}, 'Components' of entity {} is not a map", vfsPath, entry.ID);
					return false;
				}

				entries.push_back(std::move(entry));
			}

			for (const EntityEntry& entry : entries)
			{
				if (entry.Parent != 0 && !ids.contains(entry.Parent))
				{
					POLY_CORE_WARN("Cannot load world from {}, parent {} of entity {} does not exist", vfsPath, entry.Parent, entry.ID);
					return false;
				}
			}
		}
		catch (const YAML::Exception& e)
		{
			POLY_CORE_WARN("Cannot load world from {}, file is not valid yaml: {}", vfsPath, e.what());
			return false;
		}

		world.SetName(worldName);

		// Create all entities first, so parents can be resolved regardless of their order in the file
		std::unordered_map<uint64, Entity> idToEntity;
		idToEntity.reserve(entries.size());
		for (const EntityEntry& entry : entries)
			idToEntity.emplace(entry.ID, world.CreateEntity(PolyID(entry.ID)));

		for (const EntityEntry& entry : entries)
		{
			Entity& entity = idToEntity.at(entry.ID);

			// Appending in file order restores the sibling order
			if (entry.Parent != 0)
				entity.SetParent(idToEntity.at(entry.Parent));

			for (const auto& componentNode : entry.Components)
			{
				const std::string     name       = componentNode.first.as<std::string>();
				const ComponentEntry* pComponent = FindComponent(name);
				if (!pComponent)
				{
					POLY_CORE_WARN("Skipping unknown component '{}' on entity {} in {}", name, entry.ID, vfsPath);
					continue;
				}

				bool deserialized = false;
				try
				{
					deserialized = pComponent->Deserialize(componentNode.second, entity);
				}
				catch (const YAML::Exception& e)
				{
					POLY_CORE_WARN("Component '{}' on entity {} in {} is not valid yaml: {}", name, entry.ID, vfsPath, e.what());
				}

				if (!deserialized)
					POLY_CORE_WARN("Skipping component '{}' on entity {} in {}, it could not be deserialized", name, entry.ID, vfsPath);
			}
		}

		return true;
	}

	void WorldSerializer::RegisterBuiltInComponents()
	{
		RegisterComponent<NameComponent>(
		    "Name",
		    [](YAML::Emitter& out, const NameComponent& name) {
			    out << YAML::Key << "Name" << YAML::Value << name.Name;
		    },
		    [](const YAML::Node& node, NameComponent& name) {
			    name.Name = node["Name"].as<std::string>(name.Name);
			    return true;
		    });

		RegisterComponent<TransformComponent>(
		    "Transform",
		    [](YAML::Emitter& out, const TransformComponent& transform) {
			    out << YAML::Key << "Translation" << YAML::Value << transform.Translation;
			    out << YAML::Key << "Orientation" << YAML::Value << transform.Orientation;
			    out << YAML::Key << "Scale" << YAML::Value << transform.Scale;
		    },
		    [](const YAML::Node& node, TransformComponent& transform) {
			    transform.Translation = node["Translation"].as<glm::vec3>(transform.Translation);
			    transform.Orientation = node["Orientation"].as<glm::quat>(transform.Orientation);
			    transform.Scale       = node["Scale"].as<glm::vec3>(transform.Scale);
			    return true;
		    });

		RegisterComponent<MeshAssetComponent>(
		    "MeshAsset",
		    [](YAML::Emitter& out, const MeshAssetComponent& mesh) {
			    SerializeAssetPath(out, "Mesh", mesh.MeshHandle);
		    },
		    [](const YAML::Node& node, MeshAssetComponent& mesh) {
			    return DeserializeAssetPath(node, "Mesh", mesh.MeshHandle);
		    });

		RegisterComponent<MaterialComponent>(
		    "Material",
		    [](YAML::Emitter& out, const MaterialComponent& material) {
			    SerializeAssetPath(out, "Material", material.MaterialHandle);
		    },
		    [](const YAML::Node& node, MaterialComponent& material) {
			    return DeserializeAssetPath(node, "Material", material.MaterialHandle);
		    });
	}

	const WorldSerializer::ComponentEntry* WorldSerializer::FindComponent(std::string_view name) const
	{
		for (const ComponentEntry& component : m_Components)
		{
			if (component.Name == name)
				return &component;
		}

		return nullptr;
	}

	void WorldSerializer::SerializeEntity(YAML::Emitter& out, const World& world, entt::entity entity) const
	{
		auto hierarchies = world.View<IDComponent, HierarchyComponent>();

		out << YAML::BeginMap;
		out << YAML::Key << "ID" << YAML::Value << static_cast<uint64>(hierarchies.get<IDComponent>(entity).ID);

		const entt::entity parent = hierarchies.get<HierarchyComponent>(entity).Parent;
		if (parent != entt::null)
			out << YAML::Key << "Parent" << YAML::Value << static_cast<uint64>(hierarchies.get<IDComponent>(parent).ID);

		out << YAML::Key << "Components" << YAML::Value << YAML::BeginMap;
		for (const ComponentEntry& component : m_Components)
		{
			if (!component.Has(world, entity))
				continue;

			out << YAML::Key << component.Name << YAML::Value << YAML::BeginMap;
			component.Serialize(out, world, entity);
			out << YAML::EndMap;
		}
		out << YAML::EndMap;

		out << YAML::EndMap;
	}
} // namespace Poly
