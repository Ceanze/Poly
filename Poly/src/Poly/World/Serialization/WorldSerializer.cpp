#include "WorldSerializer.h"

#include "Poly/Format.h"
#include "Poly/Reflection/ComponentRegistry.h"
#include "Poly/Resources/VFS/VirtualFileSystem.h"
#include "Poly/Scene/Components.h"

#include <glaze/glaze.hpp>
#include <glaze/yaml.hpp>
#include <unordered_set>

// Glaze cannot reflect types of an anonymous namespace
namespace Poly::WorldSerializerDetail
{
	// Layout of a .polyworld file. The components of an entity are a map of component name to its fields
	struct EntityFile
	{
		uint64       ID     = 0;
		uint64       Parent = 0;
		glz::generic Components;
	};

	struct WorldFile
	{
		uint32                  Version = 0;
		std::string             World;
		std::vector<EntityFile> Entities;
	};
} // namespace Poly::WorldSerializerDetail

namespace
{
	using Poly::WorldSerializerDetail::EntityFile;
	using Poly::WorldSerializerDetail::WorldFile;

	void AppendIndented(std::string& out, std::string_view text, std::string_view indent)
	{
		while (!text.empty())
		{
			const size_t           lineEnd = text.find('\n');
			const std::string_view line    = text.substr(0, lineEnd);
			if (!line.empty())
			{
				out += indent;
				out += line;
				out += '\n';
			}

			if (lineEnd == std::string_view::npos)
				break;
			text.remove_prefix(lineEnd + 1);
		}
	}

	// Entities are written by hand rather than through EntityFile, the fields of a component are written by the
	// component itself (ComponentDesc::ToYaml) and only need to be placed at the right indentation
	// This is done to better format the file and to properly handle PolyID string to uint64 logic
	void SerializeEntity(std::string& out, const Poly::World& world, entt::entity entity)
	{
		auto hierarchies = world.View<Poly::IDComponent, Poly::HierarchyComponent>();

		out += Poly::Format("  - ID: {}\n", static_cast<uint64>(hierarchies.get<Poly::IDComponent>(entity).ID));

		const entt::entity parent = hierarchies.get<Poly::HierarchyComponent>(entity).Parent;
		if (parent != entt::null)
			out += Poly::Format("    Parent: {}\n", static_cast<uint64>(hierarchies.get<Poly::IDComponent>(parent).ID));

		std::string components;
		for (const Poly::Unique<Poly::ComponentDesc>& pComponent : Poly::ComponentRegistry::GetAll())
		{
			if (!pComponent->Has(world.GetEntity(entity)))
				continue;

			const std::string fields = pComponent->ToYaml(world.GetEntity(entity));
			if (pComponent->TypeInfo.Fields.empty() || fields.empty())
			{
				components += Poly::Format("      {}: {{}}\n", pComponent->TypeInfo.Name);
				continue;
			}

			components += Poly::Format("      {}:\n", pComponent->TypeInfo.Name);
			AppendIndented(components, fields, "        ");
		}

		if (components.empty())
		{
			out += "    Components: {}\n";
			return;
		}

		out += "    Components:\n";
		out += components;
	}
} // namespace

namespace Poly
{
	bool WorldSerializer::Save(const World& world, std::string_view vfsPath)
	{
		std::string out;
		out += Poly::Format("Version: {}\n", FORMAT_VERSION);
		out += Poly::Format("World: {}\n", glz::write_yaml(world.GetName()).value_or("\"\""));

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

		out += stack.empty() ? "Entities: []\n" : "Entities:\n";

		while (!stack.empty())
		{
			const entt::entity entity = stack.back();
			stack.pop_back();

			SerializeEntity(out, world, entity);

			const std::vector<Entity> children = world.GetEntity(entity).GetChildren();
			for (auto it = children.rbegin(); it != children.rend(); ++it)
				stack.push_back(*it);
		}

		if (!VirtualFileSystem::WriteText(vfsPath, out))
		{
			POLY_CORE_WARN("Cannot save world '{}', failed to write {}", world.GetName(), vfsPath);
			return false;
		}

		return true;
	}

	bool WorldSerializer::Load(World& world, std::string_view vfsPath)
	{
		if (!world.IsEmpty())
		{
			POLY_CORE_WARN("Cannot load world from {}, world '{}' is not empty", vfsPath, world.GetName());
			return false;
		}

		const Result<WorldData> data = Read(vfsPath);
		if (!data)
		{
			POLY_CORE_WARN("{}", data.error().Message);
			return false;
		}

		Apply(world, *data);
		return true;
	}

	Result<WorldData> WorldSerializer::Read(std::string_view vfsPath)
	{
		if (!VirtualFileSystem::Exists(vfsPath))
			return MakeError(ErrorCode::IOError, std::format("Cannot load world from {}, file cannot be found", vfsPath));

		const std::string text = VirtualFileSystem::ReadText(vfsPath);

		WorldFile file;
		if (const glz::error_ctx error = glz::read_yaml(file, text))
			return MakeError(ErrorCode::InvalidValue, std::format("Cannot load world from {}, file is not a valid world:\n{}", vfsPath, glz::format_error(error, text)));

		if (file.Version != FORMAT_VERSION)
			return MakeError(ErrorCode::InvalidValue, std::format("Cannot load world from {}, format version {} is not supported (expected {})", vfsPath, file.Version, FORMAT_VERSION));

		// Validate everything up front, so an invalid file never ends up half applied to a world
		std::unordered_set<uint64> ids;
		for (const EntityFile& entity : file.Entities)
		{
			if (entity.ID == 0 || !ids.insert(entity.ID).second)
				return MakeError(ErrorCode::InvalidValue, std::format("Cannot load world from {}, entity ID {} is missing or duplicated", vfsPath, entity.ID));

			if (!entity.Components.is_null() && !entity.Components.is_object())
				return MakeError(ErrorCode::InvalidValue, std::format("Cannot load world from {}, 'Components' of entity {} is not a map", vfsPath, entity.ID));
		}

		WorldData data;
		data.Name = std::move(file.World);
		data.Entities.reserve(file.Entities.size());

		for (const EntityFile& entity : file.Entities)
		{
			if (entity.Parent != 0 && !ids.contains(entity.Parent))
				return MakeError(ErrorCode::InvalidValue, std::format("Cannot load world from {}, parent {} of entity {} does not exist", vfsPath, entity.Parent, entity.ID));

			EntityData entityData;
			entityData.ID     = PolyID(entity.ID);
			entityData.Parent = PolyID(entity.Parent);

			if (entity.Components.is_object())
			{
				for (const auto& [name, fields] : entity.Components.get_object())
					entityData.Components.emplace_back(name, fields.is_null() ? "{}" : fields.dump().value_or("{}"));
			}

			data.Entities.push_back(std::move(entityData));
		}

		return data;
	}

	void WorldSerializer::Apply(World& world, const WorldData& data)
	{
		world.SetName(data.Name);

		// Create all entities first, so parents can be resolved regardless of their order in the data
		std::vector<Entity> entities;
		entities.reserve(data.Entities.size());
		for (const EntityData& entityData : data.Entities)
			entities.push_back(world.CreateEntity(entityData.ID));

		for (size_t i = 0; i < data.Entities.size(); i++)
		{
			const EntityData& entityData = data.Entities[i];
			Entity            entity     = entities[i];

			// Appending in data order restores the sibling order
			if (entityData.Parent != PolyID::None())
				entity.SetParent(world.FindEntity(entityData.Parent));

			ApplyComponents(entity, entityData);
		}
	}

	EntityData WorldSerializer::CaptureEntity(Entity entity)
	{
		EntityData data;
		data.ID = entity.GetPolyID();

		if (Entity parent = entity.GetParent(); parent.IsValid())
			data.Parent = parent.GetPolyID();

		for (const Unique<ComponentDesc>& pComponent : ComponentRegistry::GetAll())
		{
			if (pComponent->Has(entity))
				data.Components.emplace_back(pComponent->TypeInfo.Name, pComponent->ToJson(entity));
		}

		return data;
	}

	void WorldSerializer::ApplyComponents(Entity entity, const EntityData& data)
	{
		for (const auto& [name, json] : data.Components)
		{
			const ComponentDesc* pComponent = ComponentRegistry::Find(name);
			if (!pComponent)
			{
				POLY_CORE_WARN("Skipping unknown component '{}' on entity {}", name, data.ID);
				continue;
			}

			const bool hadComponent = pComponent->Has(entity);
			if (!hadComponent)
				pComponent->Add(entity);

			if (const Result<void> result = pComponent->FromJson(entity, json); !result)
			{
				POLY_CORE_WARN("Skipping component '{}' on entity {}, it could not be read:\n{}", name, data.ID, result.error().Message);
				if (!hadComponent)
					pComponent->Remove(entity);
			}
		}
	}
} // namespace Poly
