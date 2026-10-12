#pragma once

#include "RenderProgram.h"

#include <string>
#include <string_view>
#include <vector>

namespace Poly
{
	/*
	 * One step of a render plan, with the passes of its features already flattened: shaders, pipeline state
	 * and bindless slots are resolved, resource names are not scoped yet (see RenderProgramBuilder).
	 */
	struct RenderPlanEntry
	{
		enum class EType
		{
			FrameFeature, // Rendered once per frame, to the window
			ViewStage     // Rendered once for every view submitted to the stage
		};

		EType                     Type = EType::FrameFeature;
		std::string               Name; // Name of the feature or of the stage
		std::vector<ResolvedPass> Passes;
	};

	/*
	 * Describes everything a frame can contain, in execution order: frame features, which are rendered once
	 * every frame, and view stages, which are rendered once for every view submitted to them.
	 * Created with RenderGraph::BeginRenderPlan() and given to the renderer with Renderer::SetRenderPlan(),
	 * which compiles it together with the views submitted each frame into a RenderProgram.
	 */
	class RenderPlan
	{
	public:
		static constexpr uint32 INVALID_ENTRY = ~0u;

		explicit RenderPlan(std::vector<RenderPlanEntry> entries);

		const std::vector<RenderPlanEntry>& GetEntries() const { return m_Entries; }

		/**
		 * Finds a view stage by name
		 * @param name - name of the stage, as given to RenderPlanBuilder::AddViewStage()
		 * @return index of the entry in GetEntries(), INVALID_ENTRY if the plan has no such stage
		 */
		uint32 FindViewStage(std::string_view name) const;

	private:
		std::vector<RenderPlanEntry> m_Entries;
	};
} // namespace Poly
