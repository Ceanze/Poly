#include "RenderCatalog.h"

namespace Poly
{
	IPassDeclaration& RenderCatalog::RegisterPass(std::string_view name)
	{
		return m_PassRegistry.Register(name);
	}

	IFeatureDeclaration& RenderCatalog::RegisterFeature(std::string_view name)
	{
		return m_FeatureRegistry.Register(name);
	}

	IResourceDeclaration& RenderCatalog::RegisterResource(std::string_view name)
	{
		return m_ResourceRegistry.Register(name);
	}
} // namespace Poly
