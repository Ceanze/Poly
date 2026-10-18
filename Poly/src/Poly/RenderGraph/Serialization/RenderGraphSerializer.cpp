#include "RenderGraphSerializer.h"

#include "Poly/Reflection/GlazeEnums.h"
#include "Poly/RenderGraph/Feature/FeatureDeclaration.h"
#include "Poly/RenderGraph/Pass/PassDeclaration.h"
#include "Poly/Resources/VFS/VirtualFileSystem.h"

#include <concepts>
#include <glaze/glaze.hpp>
#include <glaze/yaml.hpp>
#include <optional>
#include <utility>

// Glaze cannot reflect types of an anonymous namespace
namespace Poly::RenderGraphSerializerDetail
{
	struct PassFile
	{
		uint32   Version = 0;
		PassDesc Pass;
	};

	struct FeatureFile
	{
		uint32      Version = 0;
		FeatureDesc Feature;
	};

	// Either a stage, with its features, or a frame feature
	struct PlanEntryFile
	{
		std::optional<std::string> Stage;
		std::optional<std::string> FrameFeature;
		std::vector<std::string>   Features;
	};

	struct PlanFile
	{
		uint32                     Version = 0;
		std::vector<PlanEntryFile> Entries;
	};

	template<typename A, typename B>
	bool IsEqual(const A& a, const B& b)
	{
		if constexpr (std::same_as<A, B>)
			return a == b;
		else
			return false;
	}

	// Leaves out every member equal to its value in a default constructed Owner when writing. Missing members keep
	// their default when reading, so nothing is lost
	template<typename Owner>
	struct SkipDefaults
	{
		template<typename T>
		static bool skip_if(T&& value, std::string_view key, const glz::meta_context&)
		{
			using V = std::decay_t<T>;
			if constexpr (!std::equality_comparable<V>)
			{
				return false;
			}
			else
			{
				static const Owner defaults{};
				const auto         tie = glz::to_tie(defaults);
				return [&]<size_t... I>(std::index_sequence<I...>) {
					return ((glz::get<I>(glz::reflect<Owner>::keys) == key && IsEqual(glz::get<I>(tie), value)) || ...);
				}(std::make_index_sequence<glz::reflect<Owner>::size>{});
			}
		}
	};
} // namespace Poly::RenderGraphSerializerDetail

namespace glz
{
	// clang-format off
	template<> struct meta<Poly::PassShader> : Poly::RenderGraphSerializerDetail::SkipDefaults<Poly::PassShader> {};
	template<> struct meta<Poly::ResourceMapping> : Poly::RenderGraphSerializerDetail::SkipDefaults<Poly::ResourceMapping> {};
	template<> struct meta<Poly::ResourceBinding> : Poly::RenderGraphSerializerDetail::SkipDefaults<Poly::ResourceBinding> {};
	template<> struct meta<Poly::ResourceUse> : Poly::RenderGraphSerializerDetail::SkipDefaults<Poly::ResourceUse> {};
	template<> struct meta<Poly::PassExecutor> : Poly::RenderGraphSerializerDetail::SkipDefaults<Poly::PassExecutor> {};
	template<> struct meta<Poly::PassDepthBias> : Poly::RenderGraphSerializerDetail::SkipDefaults<Poly::PassDepthBias> {};
	template<> struct meta<Poly::PassStencilState> : Poly::RenderGraphSerializerDetail::SkipDefaults<Poly::PassStencilState> {};
	template<> struct meta<Poly::StencilOpStateDesc> : Poly::RenderGraphSerializerDetail::SkipDefaults<Poly::StencilOpStateDesc> {};
	template<> struct meta<Poly::ColorBlendAttachmentDesc> : Poly::RenderGraphSerializerDetail::SkipDefaults<Poly::ColorBlendAttachmentDesc> {};
	template<> struct meta<Poly::PassPipelineState> : Poly::RenderGraphSerializerDetail::SkipDefaults<Poly::PassPipelineState> {};
	template<> struct meta<Poly::PassDesc> : Poly::RenderGraphSerializerDetail::SkipDefaults<Poly::PassDesc> {};
	template<> struct meta<Poly::ResourceDesc> : Poly::RenderGraphSerializerDetail::SkipDefaults<Poly::ResourceDesc> {};
	template<> struct meta<Poly::FeatureDesc> : Poly::RenderGraphSerializerDetail::SkipDefaults<Poly::FeatureDesc> {};
	template<> struct meta<Poly::RenderGraphSerializerDetail::PlanEntryFile> : Poly::RenderGraphSerializerDetail::SkipDefaults<Poly::RenderGraphSerializerDetail::PlanEntryFile> {};
	// clang-format on
} // namespace glz

namespace
{
	using namespace Poly;
	using namespace Poly::RenderGraphSerializerDetail;

	template<typename File>
	Result<std::string> WriteYaml(const File& file)
	{
		auto yaml = glz::write_yaml(file);
		if (!yaml)
			return MakeError(ErrorCode::InvalidValue, glz::format_error(yaml.error()));
		return std::move(*yaml);
	}

	template<typename File>
	Result<File> ReadYaml(std::string_view yaml, std::string_view source, std::string_view kind)
	{
		File file;
		if (const glz::error_ctx error = glz::read_yaml(file, yaml))
			return MakeError(ErrorCode::InvalidValue, std::format("Cannot read {} from {}, it is not a valid {} file:\n{}", kind, source, kind, glz::format_error(error, yaml)));

		if (file.Version != RenderGraphSerializer::FORMAT_VERSION)
			return MakeError(ErrorCode::InvalidValue, std::format("Cannot read {} from {}, format version {} is not supported (expected {})", kind, source, file.Version,
			                                                      RenderGraphSerializer::FORMAT_VERSION));

		return file;
	}

	Result<void> WriteFile(std::string_view vfsPath, const Result<std::string>& yaml, std::string_view kind)
	{
		if (!yaml)
			return MakeError(yaml.error().Code, std::format("Cannot save {} to {}: {}", kind, vfsPath, yaml.error().Message));

		if (!VirtualFileSystem::WriteText(vfsPath, *yaml))
			return MakeError(ErrorCode::IOError, std::format("Cannot save {} to {}, the file could not be written", kind, vfsPath));

		return {};
	}

	Result<std::string> ReadFile(std::string_view vfsPath, std::string_view kind)
	{
		if (!VirtualFileSystem::Exists(vfsPath))
			return MakeError(ErrorCode::IOError, std::format("Cannot read {} from {}, file cannot be found", kind, vfsPath));

		return VirtualFileSystem::ReadText(vfsPath);
	}
} // namespace

namespace Poly
{
	Result<void> RenderGraphSerializer::SavePass(const PassDeclaration& pass, std::string_view vfsPath)
	{
		if (pass.GetExecuteFn())
		{
			return MakeError(ErrorCode::InvalidValue,
			                 std::format("Cannot save pass '{}' to {}, it has an execute function. Only passes using an executor (WithExecutor) can be saved", pass.GetName(), vfsPath));
		}

		return WriteFile(vfsPath, ToYaml(pass.GetDesc()), "pass");
	}

	Result<PassDesc> RenderGraphSerializer::ReadPass(std::string_view vfsPath)
	{
		Result<std::string> yaml = ReadFile(vfsPath, "pass");
		if (!yaml)
			return std::unexpected(yaml.error());

		return PassFromYaml(*yaml, vfsPath);
	}

	Result<void> RenderGraphSerializer::SaveFeature(const FeatureDeclaration& feature, std::string_view vfsPath)
	{
		return WriteFile(vfsPath, ToYaml(feature.GetDesc()), "feature");
	}

	Result<FeatureDesc> RenderGraphSerializer::ReadFeature(std::string_view vfsPath)
	{
		Result<std::string> yaml = ReadFile(vfsPath, "feature");
		if (!yaml)
			return std::unexpected(yaml.error());

		return FeatureFromYaml(*yaml, vfsPath);
	}

	Result<void> RenderGraphSerializer::SavePlan(const RenderPlanDesc& plan, std::string_view vfsPath)
	{
		return WriteFile(vfsPath, ToYaml(plan), "plan");
	}

	Result<RenderPlanDesc> RenderGraphSerializer::ReadPlan(std::string_view vfsPath)
	{
		Result<std::string> yaml = ReadFile(vfsPath, "plan");
		if (!yaml)
			return std::unexpected(yaml.error());

		return PlanFromYaml(*yaml, vfsPath);
	}

	Result<std::string> RenderGraphSerializer::ToYaml(const PassDesc& pass)
	{
		return WriteYaml(PassFile{FORMAT_VERSION, pass});
	}

	Result<std::string> RenderGraphSerializer::ToYaml(const FeatureDesc& feature)
	{
		return WriteYaml(FeatureFile{FORMAT_VERSION, feature});
	}

	Result<std::string> RenderGraphSerializer::ToYaml(const RenderPlanDesc& plan)
	{
		PlanFile file = {.Version = FORMAT_VERSION};
		for (const RenderPlanEntryDesc& entry : plan.Entries)
		{
			if (entry.Type == ERenderPlanEntryType::ViewStage)
				file.Entries.push_back({.Stage = entry.Name, .Features = entry.Features});
			else
				file.Entries.push_back({.FrameFeature = entry.Name});
		}

		return WriteYaml(file);
	}

	Result<PassDesc> RenderGraphSerializer::PassFromYaml(std::string_view yaml, std::string_view source)
	{
		Result<PassFile> file = ReadYaml<PassFile>(yaml, source, "pass");
		if (!file)
			return std::unexpected(file.error());

		if (file->Pass.Name.empty())
			return MakeError(ErrorCode::InvalidValue, std::format("Cannot read pass from {}, it has no name", source));

		return std::move(file->Pass);
	}

	Result<FeatureDesc> RenderGraphSerializer::FeatureFromYaml(std::string_view yaml, std::string_view source)
	{
		Result<FeatureFile> file = ReadYaml<FeatureFile>(yaml, source, "feature");
		if (!file)
			return std::unexpected(file.error());

		if (file->Feature.Name.empty())
			return MakeError(ErrorCode::InvalidValue, std::format("Cannot read feature from {}, it has no name", source));

		for (const ResourceDesc& resource : file->Feature.Resources)
		{
			if (resource.Name.empty())
				return MakeError(ErrorCode::InvalidValue, std::format("Cannot read feature '{}' from {}, one of its resources has no name", file->Feature.Name, source));
		}

		return std::move(file->Feature);
	}

	Result<RenderPlanDesc> RenderGraphSerializer::PlanFromYaml(std::string_view yaml, std::string_view source)
	{
		Result<PlanFile> file = ReadYaml<PlanFile>(yaml, source, "plan");
		if (!file)
			return std::unexpected(file.error());

		RenderPlanDesc plan;
		for (size_t i = 0; i < file->Entries.size(); i++)
		{
			PlanEntryFile& entry = file->Entries[i];
			if (entry.Stage.has_value() == entry.FrameFeature.has_value())
				return MakeError(ErrorCode::InvalidValue, std::format("Cannot read plan from {}, entry {} must be either a Stage or a FrameFeature", source, i));

			if (entry.Stage)
			{
				plan.Entries.push_back({ERenderPlanEntryType::ViewStage, std::move(*entry.Stage), std::move(entry.Features)});
				continue;
			}

			if (!entry.Features.empty())
				return MakeError(ErrorCode::InvalidValue, std::format("Cannot read plan from {}, frame feature '{}' cannot have Features", source, *entry.FrameFeature));

			std::string name = std::move(*entry.FrameFeature);
			plan.Entries.push_back({ERenderPlanEntryType::FrameFeature, name, {name}});
		}

		return plan;
	}
} // namespace Poly
