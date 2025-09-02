#include "ImGuiPass.h"

#include "../Resource.h"
#include "../RenderData.h"
#include "../RenderContext.h"
#include "../RenderGraphProgram.h"
#include "Platform/API/GraphicsPipeline.h"
#include "Platform/API/Sampler.h"
#include "Platform/API/Buffer.h"
#include "Platform/API/Texture.h"
#include "Platform/API/TextureView.h"
#include "Poly/Resources/ResourceLoader.h"
#include "Poly/Resources/Shader/ShaderManager.h"
#include "Poly/Core/RenderAPI.h"
#include "Poly/Rendering/Utilities/StagingBufferCache.h"

#include <imgui.h>

namespace Poly
{
	ImGuiPass::ImGuiPass()
	{
		// TODO: Should probably not init ImGui here
		ImGui::CreateContext();
	}

	ImGuiPass::~ImGuiPass()
	{
		// TODO: Should probably not destroy ImGui here
		ImGui::DestroyContext();
	}

	PassReflection ImGuiPass::Reflect()
	{
		PassReflection reflection;

		PolyID vertShader = ShaderManager::CreateShader("shaders/imgui/ui.vert", FShaderStage::VERTEX);
		PolyID fragShader = ShaderManager::CreateShader("shaders/imgui/ui.frag", FShaderStage::FRAGMENT);

		SetShaderID(FShaderStage::VERTEX, vertShader);
		SetShaderID(FShaderStage::FRAGMENT, fragShader);

		reflection.AddShader(vertShader);
		reflection.AddShader(fragShader);

		reflection.GetField("sTexture")
			.BindPoint(FResourceBindPoint::SAMPLER | FResourceBindPoint::INTERNAL_USE)
			.Format(EFormat::R8G8B8A8_UNORM)
			.SetSampler(m_pFontSampler);

		reflection.AddPassthrough("fColor")
			.BindPoint(FResourceBindPoint::COLOR_ATTACHMENT)
			.Format(EFormat::B8G8R8A8_UNORM);

		return reflection;
	}

	void ImGuiPass::Compile()
	{
		SetupCustomPipeline();
		SetupFont();

		m_pStagingBufferCache = StagingBufferCache::Create();
	}

	void ImGuiPass::Update(const RenderContext& context)
	{
		m_BuffersToBeDestroyed[context.GetImageIndex()].clear();

		static bool first = true;
		if (first)
		{
			Ref<Resource> pRes = Resource::Create(m_pFontTexture, m_pFontTextureView, "sTexture");
			pRes->SetSampler(m_pFontSampler);
			context.GetRenderGraphProgram()->UpdateGraphResource({ "ImGuiPass.sTexture" }, pRes.get());
			first = false;
		}
		// TODO: Should probably not call Render() here
		ImGui::Render();
		UpdateBuffers(context.GetImageIndex());

		m_pStagingBufferCache->SubmitQueuedBuffers(context.GetCommandBuffer());
		m_pStagingBufferCache->Update(context.GetImageIndex());
	}

	void ImGuiPass::Execute(const RenderContext& context, const RenderData& renderData)
	{
		CommandBuffer* pCommandBuffer = context.GetCommandBuffer();

		ImGuiIO& io = ImGui::GetIO();

		// Draw
		ViewportDesc viewport = {};
		viewport.Width	= io.DisplaySize.x;
		viewport.Height	= io.DisplaySize.y;
		pCommandBuffer->SetViewport(&viewport);

		m_PushConstantData.scale		= glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
		m_PushConstantData.translate	= glm::vec2(-1.0f);
		pCommandBuffer->UpdatePushConstants(context.GetActivePipelineLayout(), FShaderStage::VERTEX, 0, sizeof(PushConstantBlock), &m_PushConstantData);

		ImDrawData* pDrawData = ImGui::GetDrawData();
		uint32 vertexOffset	= 0;
		uint32 indexOffset	= 0;
		if (pDrawData->Valid && pDrawData->CmdListsCount > 0)
		{
			pCommandBuffer->BindVertexBuffer(m_pVertexBuffer.get(), 0, 1, 0);
			pCommandBuffer->BindIndexBuffer(m_pIndexBuffer.get(), 0, EIndexType::UINT16);

			for (uint32 i = 0; i < static_cast<uint32>(pDrawData->CmdListsCount); i++)
			{
				const ImDrawList* cmdList = pDrawData->CmdLists[i];
				for (uint32 j = 0; j < static_cast<uint32>(cmdList->CmdBuffer.Size); j++)
				{
					const ImDrawCmd* pCmd = &cmdList->CmdBuffer[j];
					ScissorDesc scissor = {};
					scissor.OffsetX	= std::max(static_cast<int>(pCmd->ClipRect.x), 0);
					scissor.OffsetY	= std::max(static_cast<int>(pCmd->ClipRect.y), 0);
					scissor.Width	= static_cast<uint32>(pCmd->ClipRect.z - pCmd->ClipRect.x);
					scissor.Height	= static_cast<uint32>(pCmd->ClipRect.w - pCmd->ClipRect.y);
					pCommandBuffer->SetScissor(&scissor);

					pCommandBuffer->DrawIndexedInstanced(pCmd->ElemCount, 1, indexOffset, vertexOffset, 0);
					indexOffset += pCmd->ElemCount;
				}
				vertexOffset += cmdList->VtxBuffer.Size;
			}
		}
	}

	void ImGuiPass::SetupCustomPipeline()
	{
		Ref<GraphicsPipelineDesc> desc = CreateRef<GraphicsPipelineDesc>();

		InputAssemblyDesc inputAssembly = {};
		inputAssembly.Topology			= ETopology::TRIANGLE_LIST;
		inputAssembly.RestartPrimitive	= false;

		RasterizationDesc raster = {};
		raster.PolygonMode			= EPolygonMode::FILL;
		raster.CullMode				= ECullMode::NONE;
		raster.ClockwiseFrontFace	= true;

		ColorBlendAttachmentDesc colorBlendAttachment = {};
		colorBlendAttachment.BlendEnable			= true;
		colorBlendAttachment.ColorWriteMask			= FColorComponentFlag::RED | FColorComponentFlag::GREEN | FColorComponentFlag::BLUE | FColorComponentFlag::ALPHA;
		colorBlendAttachment.SrcColorBlendFactor	= EBlendFactor::SRC_ALPHA;
		colorBlendAttachment.DstColorBlendFactor	= EBlendFactor::ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.ColorBlendOp			= EBlendOp::ADD;
		colorBlendAttachment.SrcAlphaBlendFactor	= EBlendFactor::ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.DstAlphaBlendFactor	= EBlendFactor::ZERO;
		colorBlendAttachment.AlphaBlendOp			= EBlendOp::ADD;

		ColorBlendStateDesc blendState = {};
		blendState.ColorBlendAttachments	= { colorBlendAttachment };

		DepthStencilDesc depthStencilState = {};
		depthStencilState.DepthTestEnable	= false;
		depthStencilState.DepthWriteEnable	= false;
		depthStencilState.DepthCompareOp	= ECompareOp::LESS_OR_EQUAL;

		ViewportDesc viewport = {};
		viewport.IsDynamic = true;

		ScissorDesc scissor = {};
		scissor.IsDynamic = true;

		std::vector<VertexInput> vertexInputs;
		vertexInputs.reserve(3);

		// Position input
		VertexInput vertexInput = {};
		vertexInput.Binding			= 0;
		vertexInput.Location		= 0;
		vertexInput.Format			= EFormat::R32G32_SFLOAT;
		vertexInput.Offset			= offsetof(ImDrawVert, pos);
		vertexInput.VertexInputRate	= EVertexInputRate::VERTEX;
		vertexInput.Stride			= sizeof(ImDrawVert);
		vertexInputs.push_back(vertexInput);

		// UV input
		vertexInput.Location	= 1;
		vertexInput.Format		= EFormat::R32G32_SFLOAT;
		vertexInput.Offset		= offsetof(ImDrawVert, uv);
		vertexInputs.push_back(vertexInput);

		// Color input
		vertexInput.Location	= 2;
		vertexInput.Format		= EFormat::R8G8B8A8_UNORM;
		vertexInput.Offset		= offsetof(ImDrawVert, col);
		vertexInputs.push_back(vertexInput);

		desc->VertexInputs		= vertexInputs;
		desc->InputAssembly		= inputAssembly;
		desc->Rasterization		= raster;
		desc->ColorBlendState	= blendState;
		desc->DepthStencil		= depthStencilState;
		desc->Viewport			= viewport;
		desc->Scissor			= scissor;

		SetCustomPipelineDesc(desc);
	}

	void ImGuiPass::SetupFont()
	{
		ImGuiIO& io = ImGui::GetIO();

		// Get data
		unsigned char* fontData = nullptr;
		int width	= 0;
		int height	= 0;
		io.Fonts->GetTexDataAsRGBA32(&fontData, &width, &height);

		// Setup texture
		m_pFontTexture = ResourceLoader::LoadTextureFromMemory(fontData, width, height, 4, EFormat::R8G8B8A8_UNORM);

		// Setup texture view
		TextureViewDesc viewDesc = {};
		viewDesc.pTexture			= m_pFontTexture.get();
		viewDesc.ImageViewType		= EImageViewType::TYPE_2D;
		viewDesc.ImageViewFlag		= FImageViewFlag::SHADER_RESOURCE;
		viewDesc.Format				= EFormat::R8G8B8A8_UNORM;
		viewDesc.MipLevelCount		= 1;
		viewDesc.ArrayLayerCount	= 1;
		m_pFontTextureView = RenderAPI::CreateTextureView(&viewDesc);

		// Setup sampler
		SamplerDesc samplerDesc = {};
		samplerDesc.MagFilter		= EFilter::LINEAR;
		samplerDesc.MinFilter		= EFilter::LINEAR;
		samplerDesc.MipMapMode		= ESamplerMipmapMode::LINEAR;
		samplerDesc.AddressModeU	= ESamplerAddressMode::CLAMP_TO_EDGE;
		samplerDesc.AddressModeV	= ESamplerAddressMode::CLAMP_TO_EDGE;
		samplerDesc.AddressModeW	= ESamplerAddressMode::CLAMP_TO_EDGE;
		samplerDesc.BorderColor		= EBorderColor::FLOAT_OPAQUE_WHITE;
		m_pFontSampler = RenderAPI::CreateSampler(&samplerDesc);
	}

	void ImGuiPass::UpdateBuffers(uint32 imageIndex)
	{
		ImDrawData* pDrawData = ImGui::GetDrawData();

		uint64 vertexBufferSize	= pDrawData->TotalVtxCount * sizeof(ImDrawVert);
		uint64 indexBufferSize	= pDrawData->TotalIdxCount * sizeof(ImDrawIdx);

		// No need to update buffers if size 0, wont be used either way
		if (vertexBufferSize == 0 || indexBufferSize == 0)
			return;

		if (!m_pVertexBuffer || m_pVertexBuffer->GetSize() != vertexBufferSize)
		{
			m_BuffersToBeDestroyed[imageIndex].push_back(m_pVertexBuffer);
			BufferDesc desc = {};
			desc.BufferUsage	= FBufferUsage::TRANSFER_DST | FBufferUsage::VERTEX_BUFFER;
			desc.MemUsage		= EMemoryUsage::GPU_ONLY;
			desc.Size			= vertexBufferSize;
			m_pVertexBuffer = RenderAPI::CreateBuffer(&desc);
		}

		if (!m_pIndexBuffer || m_pIndexBuffer->GetSize() != indexBufferSize)
		{
			m_BuffersToBeDestroyed[imageIndex].push_back(m_pIndexBuffer);
			BufferDesc desc = {};
			desc.BufferUsage	= FBufferUsage::TRANSFER_DST | FBufferUsage::INDEX_BUFFER;
			desc.MemUsage		= EMemoryUsage::GPU_ONLY;
			desc.Size			= indexBufferSize;
			m_pIndexBuffer = RenderAPI::CreateBuffer(&desc);
		}

		// Update buffers with data
		uint64 vertexOffset	= 0;
		uint64 indexOffset	= 0;
		for (int i = 0; i < pDrawData->CmdListsCount; i++)
		{
			const ImDrawList* cmdList = pDrawData->CmdLists[i];
			m_pStagingBufferCache->QueueTransfer(m_pVertexBuffer.get(), cmdList->VtxBuffer.Size * sizeof(ImDrawVert), vertexOffset, cmdList->VtxBuffer.Data);
			m_pStagingBufferCache->QueueTransfer(m_pIndexBuffer.get(), cmdList->IdxBuffer.Size * sizeof(ImDrawIdx), indexOffset, cmdList->IdxBuffer.Data);
			vertexOffset += cmdList->VtxBuffer.Size * sizeof(ImDrawVert);
			indexOffset += cmdList->IdxBuffer.Size * sizeof(ImDrawIdx);
		}
	}
}