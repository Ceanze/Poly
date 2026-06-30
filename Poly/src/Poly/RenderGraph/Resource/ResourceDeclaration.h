#pragma once

#include "IResourceDeclaration.h"

namespace Poly
{
	class ResourceDeclaration : public IResourceDeclaration
	{
	public:
		ResourceDeclaration(std::string_view name);
		~ResourceDeclaration() = default;

		ResourceDeclaration& WithType(EResourceType type) override;
		ResourceDeclaration& WithInitialState(FResourceState state) override;

		std::string_view GetName() const { return m_Name; }

	private:
		const std::string m_Name;

		EResourceType  m_Type{EResourceType::None};
		FResourceState m_InitialState{FResourceState::Unknown};
	};
} // namespace Poly