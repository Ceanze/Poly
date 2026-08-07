#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class Buffer;
	class Sampler;
	class TextureView;
	class PipelineLayout;

	class DescriptorSet
	{
	public:
		CLASS_ABSTRACT(DescriptorSet);

		/**
		 * Init the Descriptor set object. Note: This also creates a pool for this set
		 * @param desc	Descriptor set creation description
		 */
		virtual void Init(PipelineLayout* pLayout, uint32 setIndex) = 0;

		/**
		 * Updates the buffer binding for this set
		 * @param binding - The binding to update
		 * @param pBuffer - The buffer to the binding to update
		 * @param offset - Offset for the region in the buffer
		 * @param range - Range for the buffer to update
		 */
		virtual void UpdateBufferBinding(uint32 binding, const Buffer* pBuffer, uint64 offset, uint64 range) = 0;

		/**
		 * Updates the texture binding for this set
		 * @param binding - The binding to update
		 * @param layout - The layout the texture will be in during the update
		 * @param pTextureView - The texture view to update
		 * @param pSampler - Sampler for the texture (ignored for a pure SAMPLED_IMAGE binding - only
		 *                   relevant for COMBINED_IMAGE_SAMPLER)
		 * @param arrayIndex - Element index to update, for an array/bindless-heap binding (0 otherwise)
		 */
		virtual void UpdateTextureBinding(uint32 binding, ETextureLayout layout, const TextureView* pTextureView, Sampler* pSampler, uint32 arrayIndex = 0) = 0;

		/**
		 * Updates a pure SAMPLER binding (as opposed to COMBINED_IMAGE_SAMPLER) - used for the
		 * bindless sampler heap, which is a separate binding from the texture heap
		 * @param binding - The binding to update
		 * @param pSampler - Sampler to write into the binding
		 * @param arrayIndex - Element index to update, for an array/bindless-heap binding (0 otherwise)
		 */
		virtual void UpdateSamplerBinding(uint32 binding, Sampler* pSampler, uint32 arrayIndex = 0) = 0;

		/**
		 * @return Native handle to the API specific object
		 */
		virtual uint64 GetNative() const = 0;

		virtual PipelineLayout* GetLayout() const = 0;

		virtual uint32 GetSetIndex() const = 0;
	};
} // namespace Poly