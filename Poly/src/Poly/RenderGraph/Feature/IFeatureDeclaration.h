#pragma once

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
	};
} // namespace Poly