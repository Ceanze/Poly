#pragma once

#include "Poly/Core/Result.h"
#include "Poly/RenderGraph/Feature/FeatureDesc.h"
#include "Poly/RenderGraph/Pass/PassDesc.h"
#include "Poly/RenderGraph/RenderPlanDesc.h"

#include <string>
#include <string_view>

namespace Poly
{
	class PassDeclaration;
	class FeatureDeclaration;

	class RenderGraphSerializer
	{
	public:
		static constexpr uint32 FORMAT_VERSION = 1;

		static constexpr std::string_view PASS_EXTENSION    = ".polypass";
		static constexpr std::string_view FEATURE_EXTENSION = ".polyfeature";
		static constexpr std::string_view PLAN_EXTENSION    = ".polyplan";

		/*
		 * Saves a pass. A pass with an execute function (IPassDeclaration::WithExecuteFn) cannot be saved, it needs an executor.
		 * @param pass The pass to save.
		 * @param vfsPath VFS path of the file to write.
		 */
		static Result<void> SavePass(const PassDeclaration& pass, std::string_view vfsPath);

		/*
		 * Reads and validates a pass file.
		 * @param vfsPath VFS path of the file to read.
		 * @return The pass, or why it could not be read.
		 */
		static Result<PassDesc> ReadPass(std::string_view vfsPath);

		/*
		 * Saves a feature, together with the resources it owns.
		 * @param feature The feature to save.
		 * @param vfsPath VFS path of the file to write.
		 */
		static Result<void> SaveFeature(const FeatureDeclaration& feature, std::string_view vfsPath);

		/*
		 * Reads and validates a feature file.
		 * @param vfsPath VFS path of the file to read.
		 * @return The feature, or why it could not be read.
		 */
		static Result<FeatureDesc> ReadFeature(std::string_view vfsPath);

		/*
		 * Saves a render plan, e.g. RenderPlanBuilder::GetDesc().
		 * @param plan The plan to save.
		 * @param vfsPath VFS path of the file to write.
		 */
		static Result<void> SavePlan(const RenderPlanDesc& plan, std::string_view vfsPath);

		/*
		 * Reads and validates a plan file. Build the plan with RenderPlanBuilder(catalog, plan).Build().
		 * @param vfsPath VFS path of the file to read.
		 * @return The plan, or why it could not be read.
		 */
		static Result<RenderPlanDesc> ReadPlan(std::string_view vfsPath);

		/*
		 * The yaml of a description, as it is written to file. For the files themselves use the Save functions.
		 */
		static Result<std::string> ToYaml(const PassDesc& pass);
		static Result<std::string> ToYaml(const FeatureDesc& feature);
		static Result<std::string> ToYaml(const RenderPlanDesc& plan);

		/*
		 * Reads a description from yaml, as written by ToYaml().
		 * @param source Where the yaml comes from, used in error messages.
		 */
		static Result<PassDesc>       PassFromYaml(std::string_view yaml, std::string_view source = "yaml");
		static Result<FeatureDesc>    FeatureFromYaml(std::string_view yaml, std::string_view source = "yaml");
		static Result<RenderPlanDesc> PlanFromYaml(std::string_view yaml, std::string_view source = "yaml");
	};
} // namespace Poly
