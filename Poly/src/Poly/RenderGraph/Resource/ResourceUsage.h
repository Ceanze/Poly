#pragma once

#include "Poly/RenderGraph/Feature/FeaturePort.h"
#include "Poly/RenderGraph/Resource/ResourceState.h"
#include "Poly/RenderGraph/Resource/ResourceType.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

#include <string>
#include <utility>
#include <vector>

namespace Poly
{
	// The layout/access/stage/aspect a resource needs to be in for a given port usage.
	// Textures use all four fields; buffers leave Layout/AspectMask at their default (unused).
	struct ResourceUsage
	{
		ETextureLayout Layout     = ETextureLayout::UNDEFINED;
		FAccessFlag    Access     = FAccessFlag::NONE;
		FPipelineStage Stage      = FPipelineStage::NONE;
		FImageViewFlag AspectMask = FImageViewFlag::NONE;
	};

	/*
	 * Determines if a resource type is image-like and carries a texture layout.
	 * @param type The resource type to check.
	 * @return True if the resource type is image-like; false if it is buffer-like.
	 */
	bool IsTextureResourceType(EResourceType type);

	/*
	 * Determines if a resource type is buffer-like and accessed via buffer device address (BDA) -
	 * i.e. it can be bound to a pointer in the push constants of a pass.
	 * @param type The resource type to check.
	 * @return True if the resource type is buffer-like and BDA-eligible; false otherwise.
	 */
	bool IsBufferResourceType(EResourceType type);

	/*
	 * Determines if a port is a semantic attachment port ("$Color"/"$Depth"/"$Stencil").
	 * @param semantic The feature port the pass port was mapped to, EFeaturePort::None if it was not.
	 * @return True if the port is a semantic attachment port; false otherwise.
	 */
	bool IsAttachmentSemanticPort(EFeaturePort semantic);

	/*
	 * ORs together the pipeline stages implied by a pass's declared shaders (vertex/fragment/compute).
	 * @param shaders The list of shaders declared for the pass.
	 * @return The combined pipeline stages.
	 */
	FPipelineStage DerivePassShaderStages(const std::vector<std::pair<std::string, FShaderStage>>& shaders);

	/*
	 * Derives the layout/access/stage/aspect a port needs to be in, based on its resource type and
	 * whether it's a semantic attachment port ("$Color"/"$Depth"/"$Stencil").
	 * @param type The resource type of the port.
	 * @param isWrite True if the port is written to; false if it is read from.
	 * @param semantic The feature port the pass port was mapped to, EFeaturePort::None if it was not.
	 * @param passShaderStages The combined pipeline stages for the pass.
	 * @return The derived resource usage.
	 */
	ResourceUsage DeriveResourceUsage(EResourceType type, bool isWrite, EFeaturePort semantic, FPipelineStage passShaderStages);

	/*
	 * Converts a declared FResourceState into the equivalent barrier-planning state. Used both to seed a
	 * resource's tracked state on first use (IResourceDeclaration::WithInitialState(), instead of assuming
	 * ETextureLayout::UNDEFINED) and to leave a render target in a specific state after its last use
	 * in the program (e.g. FResourceState::Present for a swapchain image before vkQueuePresentKHR).
	 * @param state The resource state to convert.
	 * @return The converted resource usage.
	 */
	ResourceUsage ConvertResourceState(FResourceState state);
} // namespace Poly
