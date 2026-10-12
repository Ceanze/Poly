#pragma once

#include "ResourceScope.h"
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

		/*
		 * Fixes the resource to an explicit size instead of the render target's size. Implies the
		 * resource is graph-owned/allocated (e.g. a fixed-resolution shadow map) rather than
		 * caller-supplied, since caller-supplied resources are already sized by their owner.
		 * @param width The width of the resource, in texels.
		 * @param height The height of the resource, in texels.
		 * @return Reference to the resource declaration for chaining.
		 */
		virtual IResourceDeclaration& WithSize(uint32 width, uint32 height) = 0;

		/*
		 * Sets the scope of the resource, which decides who provides it and whether views or worlds rendered
		 * by the same render program share it. Defaults to EResourceScope::Global.
		 * A resource read by the passes of a view stage as EResourceScope::View can be bound to the target of
		 * another view through RenderView::GetResources(), the reading view is then rendered after that view.
		 * @param scope The scope of the resource.
		 * @return Reference to the resource declaration for chaining.
		 */
		virtual IResourceDeclaration& WithScope(EResourceScope scope) = 0;

		virtual EResourceType  GetResourceType() const = 0;
		virtual FResourceState GetInitialState() const = 0;
		virtual EResourceScope GetScope() const        = 0;
		virtual uint32         GetWidth() const        = 0;
		virtual uint32         GetHeight() const       = 0;
		virtual bool           HasSize() const         = 0;
	};
} // namespace Poly