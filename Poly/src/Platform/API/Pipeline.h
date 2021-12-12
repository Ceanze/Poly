#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

/**
 * This pipeline is only for Graphics until Compute is added later
 */

namespace Poly
{
	struct PipelineDesc {};

	class Pipeline
	{
	public:
		CLASS_ABSTRACT(Pipeline);

		/**
		 * @return Type of pipeline
		 */
		EPipelineType GetPipelineType() const { return p_PipelineType; }

	protected:
		EPipelineType			p_PipelineType;
	};
}