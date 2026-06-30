#pragma once

#include "IFeatureDeclaration.h"

#include <string>
#include <vector>

namespace Poly
{
	class FeatureDeclaration : public IFeatureDeclaration
	{
	public:
		FeatureDeclaration(std::string_view name);
		virtual ~FeatureDeclaration() = default;

		FeatureDeclaration& WithPass(std::string_view passName) override;

		const std::vector<std::string>& GetPasses() const { return m_Passes; }

		std::string_view GetName() const override { return m_Name; }

	private:
		const std::string        m_Name;
		std::vector<std::string> m_Passes;
	};
} // namespace Poly