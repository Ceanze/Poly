#pragma once

#include "Poly/Rendering/RenderGraph/Pass.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class Buffer;

	class ReadTexturePass : public Pass
	{
	public:
		ReadTexturePass();
		~ReadTexturePass() = default;

		virtual PassReflection Reflect() override final;

		virtual void Execute(const RenderContext& context, const RenderData& renderData) override final;

		virtual void Compile() override final {};

		static Ref<ReadTexturePass> Create();

		/**
		 * Copy the staged texture data into pDst.
		 * Must be called after the GPU has finished executing the frame.
		 * @param pDst - Destination buffer to copy into
		 * @param size - Number of bytes to copy (must be <= GetDataSize())
		 */
		void CopyData(void* pDst, uint64 size) const;

		/**
		 * @return Size in bytes of the current staging buffer, or 0 if not yet allocated
		 */
		uint64 GetDataSize() const;

	private:
		Ref<Buffer> m_pStagingBuffer = nullptr;
		uint32      m_StagingWidth   = 0;
		uint32      m_StagingHeight  = 0;
	};
} // namespace Poly