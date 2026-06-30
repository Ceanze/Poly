#pragma once

#include <string_view>

namespace Poly
{
	class IFeatureDeclaration
	{
	public:
		virtual ~IFeatureDeclaration() = default;

		/*
		 * Adds a pass to the feature.
		 * The pass must be registered to the render graph before compilation.
		 */
		virtual IFeatureDeclaration& WithPass(std::string_view passName) = 0;

		/*
		 * Gets the name of the feature.
		 */
		virtual std::string_view GetName() const = 0;
	};
} // namespace Poly