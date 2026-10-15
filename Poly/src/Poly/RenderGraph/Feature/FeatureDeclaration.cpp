#include "FeatureDeclaration.h"

namespace Poly
{
	FeatureDeclaration::FeatureDeclaration(std::string_view name)
	{
		m_Desc.Name = name;
	}

	FeatureDeclaration& FeatureDeclaration::WithPass(std::string_view passName)
	{
		m_Desc.Passes.emplace_back(std::string(passName));
		return *this;
	}
} // namespace Poly
