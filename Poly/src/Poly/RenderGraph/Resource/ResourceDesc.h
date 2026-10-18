#pragma once

#include "ResourceScope.h"
#include "ResourceState.h"
#include "ResourceType.h"

#include <string>

namespace Poly
{
	/*
	 * Everything about a registered resource: what ResourceDeclaration builds up and what is written to file.
	 */
	struct ResourceDesc
	{
		std::string Name;

		EResourceType  Type         = EResourceType::None;
		FResourceState InitialState = FResourceState::Unknown;
		EResourceScope Scope        = EResourceScope::Global;

		// Explicit size in texels, 0 if the resource is sized by its owner or to the render target
		uint32 Width  = 0;
		uint32 Height = 0;

		bool operator==(const ResourceDesc&) const = default;
	};
} // namespace Poly
