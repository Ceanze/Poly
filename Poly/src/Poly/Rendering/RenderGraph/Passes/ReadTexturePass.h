#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Poly/Rendering/RenderGraph/Pass.h"

namespace Poly
{
	class Buffer;
	class SyncPoint;

	class ReadTexturePass : public Pass
	{
	public:
		ReadTexturePass();
		~ReadTexturePass() = default;

		virtual PassReflection Reflect() override final;

		virtual void Execute(RenderContext& context, const RenderData& renderData) override final;

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

		uint32 GetWidth() const { return m_StagingWidth; }
		uint32 GetHeight() const { return m_StagingHeight; }

	private:
		void AllocateStagingBuffers(uint32 width, uint32 height, EFormat format, uint32 count, uint32 imageIndex);

		std::vector<Ref<Buffer>>                             m_StagingBuffers;
		std::unordered_map<uint32, std::vector<Ref<Buffer>>> m_DeadStagingBuffers;
		uint32                                               m_StagingWidth  = 0;
		uint32                                               m_StagingHeight = 0;
		Ref<SyncPoint>                                       m_SyncPoint;
		uint64                                               m_SyncID = 0;
	};
} // namespace Poly