#include "RenderCatalog.h"

namespace Poly
{
	IPassDeclaration& RenderCatalog::RegisterPass(std::string_view name, std::string_view source)
	{
		return m_PassRegistry.Register(name, source);
	}

	IFeatureDeclaration& RenderCatalog::RegisterFeature(std::string_view name, std::string_view source)
	{
		return m_FeatureRegistry.Register(name, source);
	}

	IResourceDeclaration& RenderCatalog::RegisterResource(std::string_view name, std::string_view source)
	{
		return m_ResourceRegistry.Register(name, source);
	}

	void RenderCatalog::RemoveFromSource(std::string_view source)
	{
		m_PassRegistry.RemoveFromSource(source);
		m_FeatureRegistry.RemoveFromSource(source);
		m_ResourceRegistry.RemoveFromSource(source);
	}
} // namespace Poly
