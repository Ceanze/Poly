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
			reflect.SetBindPoint(syncData.ResourceName, syncData.SrcBindPoint);
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
				barrier.SrcAccessFlag	= ConvertToAccessFlag(data.SrcBindPoint);
				barrier.DstAccessFlag	= ConvertToAccessFlag(data.DstBindPoint);
				barrier.SrcQueueIndex	= 0; // TODO: Allow multiple queues
				barrier.DstQueueIndex	= 0;
				barrier.pBuffer			= renderData[data.ResourceName]->GetAsBuffer();
				barrier.Offset			= 0; // TODO: Allow for offset?
				bufferBarriers.push_back(barrier);
			}
			else if (data.Type == SyncType::TEXTURE)
			{
				Texture* pTexture = renderData[data.ResourceName]->GetAsTexture();
				TextureBarrier barrier = {};
				barrier.SrcAccessFlag	= ConvertToAccessFlag(data.SrcBindPoint);
				barrier.DstAccessFlag	= ConvertToAccessFlag(data.DstBindPoint);
				barrier.OldLayout		= data.SrcLayout;
				barrier.NewLayout		= data.DstLayout;
				barrier.SrcQueueIndex	= 0;
				barrier.DstQueueIndex	= 0;
				barrier.pTexture		= pTexture;
				barrier.AspectMask		= pTexture->GetDesc().Format == EFormat::D24_UNORM_S8_UINT ? FImageViewFlag::DEPTH_STENCIL : FImageViewFlag::COLOR;
				textureBarriers.push_back(barrier);

				// Update layout in resource
				renderData.GetResourceNonConst(data.ResourceName)->m_CurrentLayout = data.DstLayout;
			}
			else if (data.Type == SyncType::MEMORY)
			{
				AccessBarrier barrier = {};
				barrier.SrcAccessFlag	= ConvertToAccessFlag(data.SrcBindPoint);
				barrier.DstAccessFlag	= ConvertToAccessFlag(data.DstBindPoint);
				accessBarriers.push_back(barrier);
			}
		}

		// TODO: Deduce optimal pipeline stage?
		context.GetCommandBuffer()->PipelineBarrier(
			FPipelineStage::TOP_OF_PIPE,
			FPipelineStage::BOTTOM_OF_PIPE,
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

	FAccessFlag SyncPass::ConvertToAccessFlag(FResourceBindPoint bindPoint)
	{
		if (bindPoint == FResourceBindPoint::COLOR_ATTACHMENT)
			return FAccessFlag::COLOR_ATTACHMENT_WRITE;
		if (bindPoint == FResourceBindPoint::DEPTH_STENCIL)
			return FAccessFlag::DEPTH_STENCIL_ATTACHMENT_WRITE;
		if (bindPoint == FResourceBindPoint::UNIFORM)
			return FAccessFlag::UNIFORM_READ;
		if (bindPoint == FResourceBindPoint::SAMPLER)
			return FAccessFlag::SHADER_READ;
		if (bindPoint == FResourceBindPoint::STORAGE)
			return FAccessFlag::SHADER_READ;
		if (bindPoint == FResourceBindPoint::VERTEX)
			return FAccessFlag::VERTEX_ATTRIBUTE_READ;
		if (bindPoint == FResourceBindPoint::INDEX)
			return FAccessFlag::INDEX_READ;
		if (bindPoint == FResourceBindPoint::INDIRECT)
			return FAccessFlag::INDIRECT_COMMAND_READ;
		if (bindPoint == FResourceBindPoint::INPUT_ATTACHMENT)
			return FAccessFlag::INPUT_ATTACHMENT_READ;

		return FAccessFlag::NONE;
	}
}