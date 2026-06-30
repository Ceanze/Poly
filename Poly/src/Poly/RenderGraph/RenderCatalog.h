#pragma once

#include "Registry.h"
#include "Pass/PassDeclaration.h"
#include "Feature/FeatureDeclaration.h"
#include "Resource/ResourceDeclaration.h"

namespace Poly
{
	class IPassDeclaration;
	class IFeatureDeclaration;
	class IResourceDeclaration;

	using PassRegistry     = Registry<PassDeclaration>;
	using FeatureRegistry  = Registry<FeatureDeclaration>;
	using ResourceRegistry = Registry<ResourceDeclaration>;

	class RenderCatalog
	{
	public:
		RenderCatalog()  = default;
		~RenderCatalog() = default;

		IPassDeclaration&     RegisterPass(std::string_view name);
		IFeatureDeclaration&  RegisterFeature(std::string_view name);
		IResourceDeclaration& RegisterResource(std::string_view name);

		const PassRegistry&     GetPassRegistry() const { return m_PassRegistry; }
		const FeatureRegistry&  GetFeatureRegistry() const { return m_FeatureRegistry; }
		const ResourceRegistry& GetResourceRegistry() const { return m_ResourceRegistry; }

	private:
		PassRegistry     m_PassRegistry;
		FeatureRegistry  m_FeatureRegistry;
		ResourceRegistry m_ResourceRegistry;
	};
} // namespace Poly
