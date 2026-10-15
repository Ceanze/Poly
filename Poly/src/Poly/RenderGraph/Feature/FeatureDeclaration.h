#pragma once

#include "FeatureDesc.h"
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

		const std::vector<std::string>& GetPasses() const { return m_Desc.Passes; }

		std::string_view GetName() const override { return m_Desc.Name; }

		/*
		 * Gets the data of the feature.
		 */
		const FeatureDesc& GetDesc() const { return m_Desc; }

	private:
		FeatureDesc m_Desc;
	};
} // namespace Poly
