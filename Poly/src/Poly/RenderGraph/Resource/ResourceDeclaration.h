#pragma once

#include "IResourceDeclaration.h"
#include "ResourceDesc.h"

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
		ResourceDeclaration& WithScope(EResourceScope scope) override;

		std::string_view GetName() const { return m_Desc.Name; }

		/*
		 * Gets the data of the resource.
		 */
		const ResourceDesc& GetDesc() const { return m_Desc; }

		/*
		 * Replaces everything about the resource with a description. The name of the resource and of the description must match.
		 */
		void SetDesc(ResourceDesc desc);

		EResourceType  GetResourceType() const override { return m_Desc.Type; }
		FResourceState GetInitialState() const override { return m_Desc.InitialState; }
		EResourceScope GetScope() const override { return m_Desc.Scope; }
		uint32         GetWidth() const override { return m_Desc.Width; }
		uint32         GetHeight() const override { return m_Desc.Height; }
		bool           HasSize() const override { return m_Desc.Width != 0 && m_Desc.Height != 0; }

	private:
		ResourceDesc m_Desc;
	};
} // namespace Poly
