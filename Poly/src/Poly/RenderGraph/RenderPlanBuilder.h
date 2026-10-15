#pragma once

#include "RenderCatalog.h"
#include "RenderPlan.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace Poly
{
	class RenderPlanBuilder;

	/*
	 * Adds features to a view stage of a render plan, see RenderPlanBuilder::AddViewStage()
	 */
	class ViewStageBuilder
	{
	public:
		ViewStageBuilder(RenderPlanBuilder& planBuilder, size_t entryIndex);

		/*
		 * Adds a feature rendered for every view submitted to the stage, after the features already added.
		 * @param name The name of the feature, as registered in the render catalog.
		 * @return Reference to the builder for chaining.
		 */
		ViewStageBuilder& AddFeature(std::string_view name);

		/*
		 * Ends the stage.
		 * @return The builder of the render plan the stage belongs to.
		 */
		RenderPlanBuilder& EndStage();

	private:
		RenderPlanBuilder& m_PlanBuilder;
		size_t             m_EntryIndex;
	};

	/*
	 * Builds a RenderPlan. Frame features and view stages are rendered in the order they are added.
	 *
	 * Usage:
	 *
	 *   Ref<RenderPlan> pPlan = graph.BeginRenderPlan()
	 *                               .AddViewStage("Scene")
	 *                                   .AddFeature(GeometryFeature::FEATURE_NAME)
	 *                               .EndStage()
	 *                               .AddFrameFeature(ImGuiLayer::FEATURE_NAME)
	 *                               .Build();
	 */
	class RenderPlanBuilder
	{
	public:
		explicit RenderPlanBuilder(Ref<RenderCatalog> catalog);

		/*
		 * Continues from an existing description, e.g. one read from a plan file.
		 * @param catalog The catalog the features of the plan are registered in.
		 * @param desc The entries the plan starts with.
		 */
		RenderPlanBuilder(Ref<RenderCatalog> catalog, RenderPlanDesc desc);

		/*
		 * Adds a feature rendered once every frame to the window, whether or not any view is submitted.
		 * A frame feature has no view and no world, its passes can only use resources of EResourceScope::Global.
		 * @param name The name of the feature, as registered in the render catalog.
		 * @return Reference to the builder for chaining.
		 */
		RenderPlanBuilder& AddFrameFeature(std::string_view name);

		/*
		 * Adds a stage rendered once for every view submitted to it, see RenderViewDesc::Stage. A stage no
		 * view is submitted to renders nothing. At least one feature of a stage has to write EFeaturePort::Color.
		 * @param name The name of the stage, unique within the plan.
		 * @return A builder to add the features of the stage with.
		 */
		ViewStageBuilder AddViewStage(std::string_view name);

		/*
		 * Builds the plan. Everything the passes use has to be registered in the render catalog by now,
		 * including the resources registered by world systems.
		 * Entries that fail validation are logged and left out of the plan.
		 */
		Ref<RenderPlan> Build();

		/*
		 * Gets the description of the plan as added so far, which is the data a plan is saved as.
		 */
		const RenderPlanDesc& GetDesc() const { return m_Desc; }

	private:
		friend class ViewStageBuilder;

		// Expands the features of an entry into passes with unscoped port names. Returns false if the entry is invalid
		bool FlattenFeatures(const RenderPlanEntryDesc& entry, std::unordered_map<std::string, size_t>& featureInstanceCount, std::vector<ResolvedPass>& outPasses) const;
		// Binds the ports of each pass to the push constant fields of its shaders with the same name
		void ResolveBindlessBindings(std::vector<ResolvedPass>& passes) const;

		Ref<RenderCatalog> m_Catalog;
		RenderPlanDesc     m_Desc;
	};
} // namespace Poly
