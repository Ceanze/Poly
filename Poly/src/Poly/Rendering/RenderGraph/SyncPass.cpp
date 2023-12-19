#include "polypch.h"
#include "SyncPass.h"

#include "RenderContext.h"
#include "RenderData.h"
#include "Resource.h"
#include "Platform/API/Texture.h"

namespace Poly
{
	SyncPass::SyncPass(const std::string& name)
	{
		p_Name = name;
		p_Type = Pass::Type::SYNC;
	}

	PassReflection SyncPass::Reflect()
	{
		PassReflection reflect = {};
		// Every resource that a sync pass handles is a passthrough
		for (const auto& syncData : m_SyncData)
		{
			reflect.AddPassThrough(syncData.ResourceName);
		}

		return reflect;
	}

	void SyncPass::Execute(const RenderContext& context, const RenderData& renderData)
	{
		// When executing the command buffer has already begun
		// Since this is a SyncPass nothing  more is done before this call to the commandBuffer

		// SyncPasses change the layout (and queueIndex) if that is required, this change
		// will not be instant on the GPU but will however be instant in the Resources
		// current layout (and queueIndex), keep that in mind

		FPipelineStage srcStage = FPipelineStage::NONE;
		FPipelineStage dstStage = FPipelineStage::NONE;

		std::vector<BufferBarrier>	bufferBarriers;
		std::vector<TextureBarrier>	textureBarriers;
		std::vector<AccessBarrier>	accessBarriers;
		for (auto& data : m_SyncData)
		{
			// If type wasn't given, deduce what it might be (given that the other things are correct)
			if (data.Type == SyncType::NONE)
			{
				if (data.DstLayout != ETextureLayout::UNDEFINED)
					data.Type = SyncType::TEXTURE;
				else if (data.ResourceName.empty())
					data.Type = SyncType::MEMORY;
				else
					data.Type = SyncType::BUFFER;
			}

			if (data.Type == SyncType::BUFFER)
			{
				BufferBarrier barrier = {};
				barrier.SrcAccessFlag	= data.SrcAccessFlag;
				barrier.DstAccessFlag	= data.DstAccessFlag;
				barrier.SrcQueueIndex	= 0; // TODO: Allow multiple queues
				barrier.DstQueueIndex	= 0;
				barrier.pBuffer			= renderData[data.ResourceName]->GetAsBuffer();
				barrier.Offset			= 0; // TODO: Allow for offset?
				bufferBarriers.push_back(barrier);

				srcStage |= data.SrcPipelineStage;
				dstStage |= data.DstPipelineStage;
			}
			else if (data.Type == SyncType::TEXTURE)
			{
				Texture* pTexture = renderData[data.ResourceName]->GetAsTexture();
				TextureBarrier barrier = {};
				barrier.SrcAccessFlag	= data.SrcAccessFlag;
				barrier.DstAccessFlag	= data.DstAccessFlag;
				barrier.OldLayout		= data.SrcLayout;
				barrier.NewLayout		= data.DstLayout;
				barrier.SrcQueueIndex	= 0;
				barrier.DstQueueIndex	= 0;
				barrier.pTexture		= pTexture;
				barrier.AspectMask		= pTexture->GetDesc().Format == EFormat::DEPTH_STENCIL ? FImageViewFlag::DEPTH_STENCIL : FImageViewFlag::COLOR; // TODO: Handle either cusom options or more gradual choises
				textureBarriers.push_back(barrier);

				srcStage |= data.SrcPipelineStage;
				dstStage |= data.DstPipelineStage;

				// Update layout in resource
				renderData.GetResourceNonConst(data.ResourceName)->m_CurrentLayout = data.DstLayout;
			}
			else if (data.Type == SyncType::MEMORY)
			{
				AccessBarrier barrier = {};
				barrier.SrcAccessFlag	= data.SrcAccessFlag;
				barrier.DstAccessFlag	= data.DstAccessFlag;
				accessBarriers.push_back(barrier);

				srcStage |= data.SrcPipelineStage;
				dstStage |= data.DstPipelineStage;
			}
		}

		// TODO: Deduce optimal pipeline stage?
		context.GetCommandBuffer()->PipelineBarrier(
			srcStage,
			dstStage,
			accessBarriers,
			bufferBarriers,
			textureBarriers);

	}

	void SyncPass::AddSyncData(SyncData syncData)
	{
		// Sync pass should not contain too many resources, search should therefore be quite fast
		auto it = std::find(m_SyncData.begin(), m_SyncData.end(), syncData);
		if (it == m_SyncData.end())
			m_SyncData.push_back(syncData);
		else
			POLY_CORE_WARN("AddSyncData for resource {} could not be done, resource has already been added to {}", syncData.ResourceName, p_Name);
	}

	Ref<SyncPass> SyncPass::Create(const std::string& name)
	{
		return CreateRef<SyncPass>(name);
	}
}
