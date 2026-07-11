#include "ResourceDeclaration.h"

namespace Poly
{
	ResourceDeclaration::ResourceDeclaration(std::string_view name)
	    : m_Name(name)
	{}

	ResourceDeclaration& ResourceDeclaration::WithType(EResourceType type)
	{
		m_Type = type;
		return *this;
	}

	ResourceDeclaration& ResourceDeclaration::WithInitialState(FResourceState state)
	{
		m_InitialState = state;
		return *this;
	}

	ResourceDeclaration& ResourceDeclaration::WithSize(uint32 width, uint32 height)
	{
		m_Width  = width;
		m_Height = height;
		return *this;
	}
} // namespace Poly