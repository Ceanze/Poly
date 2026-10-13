#pragma once

namespace Poly
{
	// Decides who provides a resource and how many instances of it a render program has
	enum class EResourceScope
	{
		Global, // One for the whole program, provided by Renderer::GetGlobalResources()
		World,  // One per world being rendered, provided by the world's RenderResourceTable
		View    // One per view being rendered, provided by the renderer or by RenderView::GetResources()
	};
} // namespace Poly
