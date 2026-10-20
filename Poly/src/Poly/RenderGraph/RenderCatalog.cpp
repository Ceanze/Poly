#include "RenderCatalog.h"

#include "Poly/Core/EnumNames.h"
#include "Poly/RenderGraph/Serialization/RenderGraphSerializer.h"
#include "Poly/Resources/VFS/VirtualFileSystem.h"

#include <algorithm>
#include <unordered_set>

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

	using namespace Poly;

	class IssueList
	{
	public:
		IssueList(std::vector<CatalogIssue>& issues, std::string_view source, std::string prefix)
		    : m_Issues(issues)
		    , m_Source(source)
		    , m_Prefix(std::move(prefix))
		{}

		void Error(std::string message) { Add(CatalogIssue::ESeverity::Error, std::move(message)); }
		void Warning(std::string message) { Add(CatalogIssue::ESeverity::Warning, std::move(message)); }

	private:
		void Add(CatalogIssue::ESeverity severity, std::string message)
		{
			m_Issues.push_back({severity, std::string(m_Source), m_Prefix + std::move(message)});
		}

		std::vector<CatalogIssue>& m_Issues;
		std::string_view           m_Source;
		std::string                m_Prefix;
	};

	void ValidatePass(const RenderCatalog& catalog, const PassDeclaration& pass, IssueList issues)
	{
		const PassDesc& desc = pass.GetDesc();

		if (desc.Shaders.empty())
			issues.Error("has no shaders");

		FShaderStage stages = FShaderStage::NONE;
		for (const PassShader& shader : desc.Shaders)
		{
			if (!VirtualFileSystem::Exists(shader.Path))
				issues.Error(std::format("shader file '{}' does not exist", shader.Path));

			if (BitsSet(stages, shader.Stage))
				issues.Error(std::format("has more than one {} shader", ToString(shader.Stage)));
			stages |= shader.Stage;
		}

		if (!desc.Executor.Name.empty())
		{
			if (Result<ExecuteFn> executeFn = catalog.GetExecutorRegistry().Create(desc.Executor.Name, desc.Executor.ParamsJson); !executeFn)
				issues.Error(executeFn.error().Message);
		}
		else if (!pass.GetExecuteFn())
		{
			issues.Warning("has neither an executor nor an execute function, it records no draws");
		}

		if (!desc.VertexLayoutName.empty() && !catalog.GetVertexLayoutRegistry().Get(desc.VertexLayoutName))
			issues.Error(std::format("vertex layout '{}' is not registered", desc.VertexLayoutName));

		for (const ResourceMapping& mapping : desc.ResourceMappings)
		{
			if (mapping.Port == EFeaturePort::None)
				issues.Error(std::format("shader resource '{}' is mapped to no feature port", mapping.ShaderResourceName));
		}

		// An unregistered name is a transient texture of the render program, which nothing writes when it is only read
		for (const ResourceBinding& global : desc.GlobalMappings)
		{
			if (!catalog.GetResourceRegistry().Get(global.ResourceName))
				issues.Warning(std::format("global '{}' is not a registered resource, the pass reads a texture nothing writes", global.ResourceName));
		}

		for (const ResourceUse& use : desc.ResourceUses)
		{
			if (!catalog.GetResourceRegistry().Get(use.Name))
				issues.Error(std::format("{} resource '{}' is not registered", use.IsWrite ? "written" : "read", use.Name));
		}
	}

	void ValidateFeature(const RenderCatalog& catalog, const FeatureDesc& desc, IssueList issues)
	{
		if (desc.Passes.empty())
			issues.Warning("has no passes");

		for (const std::string& passName : desc.Passes)
		{
			if (!catalog.GetPassRegistry().Get(passName))
				issues.Error(std::format("pass '{}' is not registered", passName));
		}

		for (const ResourceDesc& resource : desc.Resources)
		{
			if (resource.Type == EResourceType::None)
				issues.Error(std::format("resource '{}' has no type", resource.Name));
		}
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

		// Only the problems of files are reported, those of code registrations are not for the files to fix
		for (const CatalogIssue& issue : Validate())
		{
			if (issue.Source.empty())
				continue;

			if (issue.Severity == CatalogIssue::ESeverity::Error)
			{
				POLY_CORE_ERROR("{}: {}", issue.Source, issue.Message);
				allLoaded = false;
			}
			else
			{
				POLY_CORE_WARN("{}: {}", issue.Source, issue.Message);
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

	std::vector<CatalogIssue> RenderCatalog::Validate() const
	{
		std::vector<CatalogIssue> issues;

		for (const auto& [name, entry] : m_PassRegistry.GetAll())
			ValidatePass(*this, entry.Value, IssueList(issues, entry.Source, std::format("Pass '{}': ", name)));

		for (const auto& [name, entry] : m_FeatureRegistry.GetAll())
			ValidateFeature(*this, entry.Value.GetDesc(), IssueList(issues, entry.Source, std::format("Feature '{}': ", name)));

		return issues;
	}

	std::vector<CatalogIssue> RenderCatalog::ValidatePlan(const RenderPlanDesc& plan) const
	{
		std::vector<CatalogIssue>       issues;
		std::unordered_set<std::string> stageNames;

		for (const RenderPlanEntryDesc& entry : plan.Entries)
		{
			const bool isStage = entry.Type == ERenderPlanEntryType::ViewStage;
			IssueList  entryIssues(issues, {}, std::format("{} '{}': ", isStage ? "Stage" : "Frame feature", entry.Name));

			if (isStage && !stageNames.insert(entry.Name).second)
				entryIssues.Error("is in the plan more than once");

			if (isStage && entry.Features.empty())
				entryIssues.Warning("has no features");

			if (!isStage && (entry.Features.size() != 1 || entry.Features.front() != entry.Name))
				entryIssues.Error("must have exactly one feature, itself");

			for (const std::string& featureName : entry.Features)
			{
				if (!m_FeatureRegistry.Get(featureName))
					entryIssues.Error(std::format("feature '{}' is not registered", featureName));
			}
		}

		return issues;
	}

	void RenderCatalog::RemoveFromSource(std::string_view source)
	{
		m_PassRegistry.RemoveFromSource(source);
		m_FeatureRegistry.RemoveFromSource(source);
		m_ResourceRegistry.RemoveFromSource(source);
	}
} // namespace Poly
