#include "WorldService.h"

#include "Poly/API/EngineContext.h"
#include "Poly/World/Serialization/WorldSerializer.h"

namespace Poly::API
{
	WorldService::WorldService(EngineContext& context)
	    : m_Context(context)
	{}

	void WorldService::New(std::string_view name)
	{
		World& world = m_Context.GetWorld();
		world.Clear();
		world.SetName(name);

		m_Path.clear();

		// The commands refer to entities that no longer exist
		m_Context.History().Clear();
		m_Context.Events().Queue(WorldCleared{std::string(name)});
	}

	Result<void> WorldService::Load(std::string_view vfsPath)
	{
		// Read before clearing, so a file that cannot be loaded leaves the current world as it is
		Result<WorldData> data = WorldSerializer::Read(vfsPath);
		if (!data)
			return std::unexpected(data.error());

		World& world = m_Context.GetWorld();
		world.Clear();
		WorldSerializer::Apply(world, *data);

		m_Path = vfsPath;

		m_Context.History().Clear();
		m_Context.Events().Queue(WorldLoaded{world.GetName(), m_Path});
		return {};
	}

	Result<void> WorldService::Save(std::string_view vfsPath)
	{
		const std::string path(vfsPath.empty() ? std::string_view(m_Path) : vfsPath);
		if (path.empty())
			return MakeError(ErrorCode::IOError, "Cannot save world, it has never been saved or loaded so a path is needed");

		if (!WorldSerializer::Save(m_Context.GetWorld(), path))
			return MakeError(ErrorCode::IOError, fmt::format("Cannot save world, failed to write {}", path));

		m_Path = path;

		m_Context.History().MarkSaved();
		m_Context.Events().Queue(WorldSaved{m_Path});
		return {};
	}

	const std::string& WorldService::GetName() const
	{
		return m_Context.GetWorld().GetName();
	}

	bool WorldService::IsDirty() const
	{
		return m_Context.History().IsDirty();
	}
} // namespace Poly::API
