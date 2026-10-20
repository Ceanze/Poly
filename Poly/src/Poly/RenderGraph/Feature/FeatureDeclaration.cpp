#include "FeatureDeclaration.h"

namespace Poly
{
	FeatureDeclaration::FeatureDeclaration(std::string_view name)
	{
		m_Desc.Name = name;
	}

	void FeatureDeclaration::SetDesc(FeatureDesc desc)
	{
		POLY_VALIDATE(desc.Name == m_Desc.Name, "Description of feature '{}' is for feature '{}'", m_Desc.Name, desc.Name);
		m_Desc = std::move(desc);
	}

	FeatureDeclaration& FeatureDeclaration::WithPass(std::string_view passName)
	{
		m_Desc.Passes.emplace_back(std::string(passName));
		return *this;
	}
} // namespace Poly
