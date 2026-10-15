#pragma once

#include "Poly/RenderGraph/Resource/ResourceDesc.h"

#include <string>
#include <vector>

namespace Poly
{
	/*
	 * Everything about a feature: what FeatureDeclaration builds up and what is written to a feature file.
	 */
	struct FeatureDesc
	{
		std::string Name;

		// Names of the passes of the feature, in execution order
		std::vector<std::string> Passes;

		// Resources the feature owns, for the ones no system or other code registers (e.g. a shadow map)
		std::vector<ResourceDesc> Resources;
	};
} // namespace Poly
