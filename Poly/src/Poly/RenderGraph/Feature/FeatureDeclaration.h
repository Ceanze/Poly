#pragma once

#include "IFeatureDeclaration.h"

namespace Poly
{
	class FeatureDeclaration : public IFeatureDeclaration
	{
	public:
		FeatureDeclaration()          = default;
		virtual ~FeatureDeclaration() = default;

		FeatureDeclaration& WithPass(std::string_view passName) override;

		const std::vector<std::string>& GetPasses() const { return m_Passes; }

	private:
		std::vector<std::string> m_Passes;
	};
} // namespace Poly