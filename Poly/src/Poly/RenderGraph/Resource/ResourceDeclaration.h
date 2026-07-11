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
		ResourceDeclaration& WithSize(uint32 width, uint32 height) override;

		std::string_view GetName() const { return m_Name; }

		EResourceType  GetType() const override { return m_Type; }
		FResourceState GetInitialState() const override { return m_InitialState; }
		uint32         GetWidth() const override { return m_Width; }
		uint32         GetHeight() const override { return m_Height; }
		bool           HasSize() const override { return m_Width != 0 && m_Height != 0; }

	private:
		const std::string m_Name;

		EResourceType  m_Type{EResourceType::None};
		FResourceState m_InitialState{FResourceState::Unknown};
		uint32         m_Width  = 0;
		uint32         m_Height = 0;
	};
} // namespace Poly