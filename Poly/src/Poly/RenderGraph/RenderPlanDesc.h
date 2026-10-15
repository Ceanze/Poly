#pragma once

#include <string>
#include <vector>

namespace Poly
{
	enum class ERenderPlanEntryType
	{
		FrameFeature, // Rendered once per frame, to the window
		ViewStage     // Rendered once for every view submitted to the stage
	};

	// One step of a render plan, by the names of its features
	struct RenderPlanEntryDesc
	{
		ERenderPlanEntryType Type = ERenderPlanEntryType::FrameFeature;

		// Name of the stage, or of the feature for a frame feature
		std::string Name;

		// Names of the features of the entry, in execution order. A frame feature has exactly one, itself
		std::vector<std::string> Features;
	};

	struct RenderPlanDesc
	{
		std::vector<RenderPlanEntryDesc> Entries;
	};
} // namespace Poly
