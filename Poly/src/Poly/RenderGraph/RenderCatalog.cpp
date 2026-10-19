#include "RenderCatalog.h"

#include "Poly/RenderGraph/Serialization/RenderGraphSerializer.h"
#include "Poly/Resources/VFS/VirtualFileSystem.h"

#include <algorithm>

namespace
{
	// "assets/render/passes/pbr.polypass" -> "pbr"
	std::string_view FileStem(std::string_view path)
	{
		const size_t slash = path.find_last_of("/\\");
		if (slash != std::string_view::npos)
			path.remove_prefix(slash + 1);
		return path.substr(0, path.find('.'));
	}

	template<typename T>
	Poly::Result<void> CheckNameFree(const Poly::Registry<T>& registry, std::string_view kind, std::string_view name, std::string_view vfsPath)
	{
		const auto* pEntry = registry.GetEntry(name);
		if (!pEntry || pEntry->Source == vfsPath)
			return {};

		return Poly::MakeError(Poly::ErrorCode::InvalidValue, std::format("{} declares the {} '{}', which is already registered by {}. The file is not registered", vfsPath, kind, name,
		                                                                  pEntry->Source.empty() ? "code" : pEntry->Source));
	}

	void WarnIfNameDiffers(std::string_view vfsPath, std::string_view name)
	{
		if (FileStem(vfsPath) != name)
			POLY_CORE_WARN("{} declares '{}', the file is easier to find if it is named after it", vfsPath, name);
	}
} // namespace

namespace Poly
{
	IPassDeclaration& RenderCatalog::RegisterPass(std::string_view name, std::string_view source)
	{
		return m_PassRegistry.Register(name, source);
	}

	IFeatureDeclaration& RenderCatalog::RegisterFeature(std::string_view name, std::string_view source)
	{
		return m_FeatureRegistry.Register(name, source);
	}

	IResourceDeclaration& RenderCatalog::RegisterResource(std::string_view name, std::string_view source)
	{
		return m_ResourceRegistry.Register(name, source);
	}

	bool RenderCatalog::LoadDirectory(std::string_view vfsDirectory)
	{
		std::vector<std::string> files = VirtualFileSystem::EnumerateFiles(vfsDirectory);
		std::ranges::sort(files);

		bool allLoaded = true;
		for (const std::string& file : files)
		{
			if (!file.ends_with(RenderGraphSerializer::PASS_EXTENSION) && !file.ends_with(RenderGraphSerializer::FEATURE_EXTENSION))
				continue;

			if (Result<void> result = LoadFile(file); !result)
			{
				POLY_CORE_ERROR("{}", result.error().Message);
				allLoaded = false;
			}
		}

		return allLoaded;
	}

	Result<void> RenderCatalog::LoadFile(std::string_view vfsPath)
	{
		if (vfsPath.ends_with(RenderGraphSerializer::PASS_EXTENSION))
		{
			Result<PassDesc> pass = RenderGraphSerializer::ReadPass(vfsPath);
			if (!pass)
				return std::unexpected(pass.error());

			if (Result<void> free = CheckNameFree(m_PassRegistry, "pass", pass->Name, vfsPath); !free)
				return free;

			WarnIfNameDiffers(vfsPath, pass->Name);
			RemoveFromSource(vfsPath);
			m_PassRegistry.Register(pass->Name, vfsPath).SetDesc(std::move(*pass));
			return {};
		}

		if (vfsPath.ends_with(RenderGraphSerializer::FEATURE_EXTENSION))
		{
			Result<FeatureDesc> feature = RenderGraphSerializer::ReadFeature(vfsPath);
			if (!feature)
				return std::unexpected(feature.error());

			if (Result<void> free = CheckNameFree(m_FeatureRegistry, "feature", feature->Name, vfsPath); !free)
				return free;

			for (const ResourceDesc& resource : feature->Resources)
			{
				if (Result<void> free = CheckNameFree(m_ResourceRegistry, "resource", resource.Name, vfsPath); !free)
					return free;
			}

			WarnIfNameDiffers(vfsPath, feature->Name);
			RemoveFromSource(vfsPath);
			for (const ResourceDesc& resource : feature->Resources)
				m_ResourceRegistry.Register(resource.Name, vfsPath).SetDesc(resource);
			m_FeatureRegistry.Register(feature->Name, vfsPath).SetDesc(std::move(*feature));
			return {};
		}

		return MakeError(ErrorCode::InvalidValue, std::format("{} is neither a pass ({}) nor a feature ({}) file", vfsPath, RenderGraphSerializer::PASS_EXTENSION,
		                                                      RenderGraphSerializer::FEATURE_EXTENSION));
	}

	void RenderCatalog::RemoveFromSource(std::string_view source)
	{
		m_PassRegistry.RemoveFromSource(source);
		m_FeatureRegistry.RemoveFromSource(source);
		m_ResourceRegistry.RemoveFromSource(source);
	}
} // namespace Poly
