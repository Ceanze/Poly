#include "ResourceDeclaration.h"

namespace Poly
{
	ResourceDeclaration::ResourceDeclaration(std::string_view name)
	{
		m_Desc.Name = name;
	}

	void ResourceDeclaration::SetDesc(ResourceDesc desc)
	{
		POLY_VALIDATE(desc.Name == m_Desc.Name, "Description of resource '{}' is for resource '{}'", m_Desc.Name, desc.Name);
		m_Desc = std::move(desc);
	}

	ResourceDeclaration& ResourceDeclaration::WithType(EResourceType type)
	{
		m_Desc.Type = type;
		return *this;
	}

	ResourceDeclaration& ResourceDeclaration::WithInitialState(FResourceState state)
	{
		m_Desc.InitialState = state;
		return *this;
	}

	ResourceDeclaration& ResourceDeclaration::WithSize(uint32 width, uint32 height)
	{
		m_Desc.Width  = width;
		m_Desc.Height = height;
		return *this;
	}

	ResourceDeclaration& ResourceDeclaration::WithScope(EResourceScope scope)
	{
		m_Desc.Scope = scope;
		return *this;
	}
} // namespace Poly
