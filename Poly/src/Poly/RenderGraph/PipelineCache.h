#pragma once

#include "Poly/Core/Core.h"
#include "RenderProgram.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace Poly
{
	class PipelineLayout;
	class GraphicsPipeline;

	/*
	 * Owns the pipelines and pipeline layouts of the passes of a render plan. A pipeline only depends on the
	 * pass and the formats it renders to, not on the render program the pass is part of, so keeping them here
	 * makes switching between render programs free of pipeline creation.
	 * Not thread safe, pipelines are requested before passes are recorded in parallel.
	 */
	class PipelineCache
	{
	public:
		struct Entry
		{
			PipelineLayout*   pLayout   = nullptr;
			GraphicsPipeline* pPipeline = nullptr;
		};

		PipelineCache()  = default;
		~PipelineCache() = default;
		CLASS_REMOVE_COPY(PipelineCache);

		/**
		 * Gets the pipeline of a pass for the given attachment formats, creating it the first time it is asked for
		 * @param pass - pass to get the pipeline of, identified by its name
		 * @param colorFormats - formats of the color attachments the pass writes
		 * @param depthFormat - format of the depth attachment, UNDEFINED if the pass has none
		 * @param stencilFormat - format of the stencil attachment, UNDEFINED if the pass has none
		 */
		Entry GetOrCreate(const ResolvedPass& pass, const std::vector<EFormat>& colorFormats, EFormat depthFormat, EFormat stencilFormat);

		/**
		 * Destroys all pipelines and layouts. The caller makes sure the GPU no longer uses them
		 */
		void Clear();

	private:
		PipelineLayout* GetOrCreateLayout(const ResolvedPass& pass);

		std::unordered_map<std::string, Ref<PipelineLayout>>   m_Layouts;   // keyed by pass name
		std::unordered_map<std::string, Ref<GraphicsPipeline>> m_Pipelines; // keyed by pass name and attachment formats
	};
} // namespace Poly
