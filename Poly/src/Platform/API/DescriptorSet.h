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
		virtual void UpdateBufferBinding(uint32 binding, Buffer* pBuffer, uint64 offset, uint64 range) = 0;

		/**
		 * Updates the texture binding for this set
		 * @param binding - The binding to update
		 * @param layout - The layout the texture will be in during the update
		 * @param pTextureView - The texture view to update
		 * @param pSampler - Sampler for the texture
		 */
		virtual void UpdateTextureBinding(uint32 binding, ETextureLayout layout, TextureView* pTextureView, Sampler* pSampler) = 0;

		/**
		 * @return Native handle to the API specific object
		 */
		virtual uint64 GetNative() const = 0;
	};
}