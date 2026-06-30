#pragma once

#include "ResourceState.h"
#include "ResourceType.h"

namespace Poly
{
	class IResourceDeclaration
	{
	public:
		virtual ~IResourceDeclaration() = default;

		/*
		 * Sets the type of the resource. Required for the render graph to handle descriptors.
		 * @param type The type of the resource.
		 * @return Reference to the resource declaration for chaining.
		 */
		virtual IResourceDeclaration& WithType(EResourceType type) = 0;

		/*
		 * Sets the initial state of the resource. This is used by the render graph to determine the necessary resource barriers.
		 * @param state The initial state of the resource.
		 * @return Reference to the resource declaration for chaining.
		 */
		virtual IResourceDeclaration& WithInitialState(FResourceState state) = 0;
	};
} // namespace Poly