#include "polypch.h"
#include "PVKGraphicsPipeline.h"
#include "PVKInstance.h"
#include "VulkanCommon.h"
#include "PVKShader.h"
#include "PVKPipelineLayout.h"
#include "PVKRenderPass.h"

// TODO: Make the pipeline support both compute and graphcis pipelines

namespace Poly
{

	PVKGraphicsPipeline::~PVKGraphicsPipeline()
	{
		vkDestroyPipeline(PVKInstance::GetDevice(), m_Pipeline, nullptr);
	}

	void PVKGraphicsPipeline::Init(const GraphicsPipelineDesc* pDesc)
	{
		m_pPipelineLayout = reinterpret_cast<PVKPipelineLayout*>(pDesc->pPipelineLayout);
		p_PipelineType = EPipelineType::GRAPHICS;

		// Create vertex arrays
		VkVertexInputBindingDescription vertexBinding = {};
		std::vector<VkVertexInputAttributeDescription> vertexAttribs;
		vertexAttribs.reserve(pDesc->VertexInputs.size());

		if (!pDesc->VertexInputs.empty())
		{
			vertexBinding.binding	= pDesc->VertexInputs[0].Binding;
			vertexBinding.stride	= pDesc->VertexInputs[0].Stride;
			vertexBinding.inputRate	= ConvertVertexInputRateVK(pDesc->VertexInputs[0].VertexInputRate);
		}

		for (auto& vertexInput : pDesc->VertexInputs)
		{
			VkVertexInputAttributeDescription attribDesc = {};
			attribDesc.binding	= vertexInput.Binding;
			attribDesc.location	= vertexInput.Location;
			attribDesc.format	= ConvertFormatVK(vertexInput.Format);
			attribDesc.offset	= vertexInput.Offset;

			vertexAttribs.push_back(attribDesc);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.pVertexBindingDescriptions		= vertexAttribs.empty() ? nullptr : &vertexBinding;
		vertexInputInfo.vertexBindingDescriptionCount	= vertexAttribs.empty() ? 0 : 1;
		vertexInputInfo.pVertexAttributeDescriptions	= vertexAttribs.data();
		vertexInputInfo.vertexAttributeDescriptionCount	= static_cast<uint32>(vertexAttribs.size());
		vertexInputInfo.flags							= 0;
		vertexInputInfo.pNext							= nullptr;

		// Input assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType						= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology					= ConvertTopologyVK(pDesc->InputAssembly.Topology);
		inputAssembly.primitiveRestartEnable	= pDesc->InputAssembly.RestartPrimitive;

		// Viewport [Only supports one currently]
		VkViewport viewport = {};
		viewport.x			= pDesc->Viewport.PosX;
		viewport.y			= pDesc->Viewport.PosY;
		viewport.width		= pDesc->Viewport.Width;
		viewport.height		= pDesc->Viewport.Height;
		viewport.minDepth	= pDesc->Viewport.MinDepth;
		viewport.maxDepth	= pDesc->Viewport.MaxDepth;

		// Scissor [Only supports one currently]
		VkRect2D scissor = {};
		scissor.extent = { pDesc->Scissor.Width, pDesc->Scissor.Height };
		scissor.offset = { pDesc->Scissor.OffsetX, pDesc->Scissor.OffsetY };

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType			= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount	= 1;
		viewportState.pViewports	= &viewport;
		viewportState.scissorCount	= 1;
		viewportState.pScissors		= &scissor;

		// Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType					= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable			= pDesc->Rasterization.DepthClampEnable;
		rasterizer.rasterizerDiscardEnable	= pDesc->Rasterization.DiscardEnable;
		rasterizer.polygonMode				= ConvertPolygonModeVK(pDesc->Rasterization.PolygonMode);
		rasterizer.lineWidth				= pDesc->Rasterization.LineWidth;
		rasterizer.cullMode					= ConvertCullModeVK(pDesc->Rasterization.CullMode);
		rasterizer.frontFace				= pDesc->Rasterization.ClockwiseFrontFace ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable			= pDesc->Rasterization.DepthBiasEnable;
		rasterizer.depthBiasConstantFactor	= pDesc->Rasterization.DepthBiasFactor;
		rasterizer.depthBiasClamp			= pDesc->Rasterization.DepthBiasClamp;
		rasterizer.depthBiasSlopeFactor		= pDesc->Rasterization.DepthBiasSlopeFactor;

		// Multisampling
		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType					= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable	= VK_FALSE;
		multisampling.rasterizationSamples	= VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading		= 1.0f;
		multisampling.pSampleMask			= nullptr;
		multisampling.alphaToCoverageEnable	= VK_FALSE;
		multisampling.alphaToOneEnable		= VK_FALSE;

		// Depth Stencil
		VkStencilOpState frontState = {};
		frontState.failOp		= ConvertStencilOpVK(pDesc->DepthStencil.Front.FailOp);
		frontState.passOp		= ConvertStencilOpVK(pDesc->DepthStencil.Front.PassOp);
		frontState.depthFailOp	= ConvertStencilOpVK(pDesc->DepthStencil.Front.DepthFailOp);
		frontState.compareOp	= ConvertCompareOpVK(pDesc->DepthStencil.Front.CompareOp);
		frontState.compareMask	= pDesc->DepthStencil.Front.CompareMask;
		frontState.writeMask	= pDesc->DepthStencil.Front.WriteMask;
		frontState.reference	= pDesc->DepthStencil.Front.Reference;

		VkStencilOpState backState = {};
		backState.failOp		= ConvertStencilOpVK(pDesc->DepthStencil.Back.FailOp);
		backState.passOp		= ConvertStencilOpVK(pDesc->DepthStencil.Back.PassOp);
		backState.depthFailOp	= ConvertStencilOpVK(pDesc->DepthStencil.Back.DepthFailOp);
		backState.compareOp		= ConvertCompareOpVK(pDesc->DepthStencil.Back.CompareOp);
		backState.compareMask	= pDesc->DepthStencil.Back.CompareMask;
		backState.writeMask		= pDesc->DepthStencil.Back.WriteMask;
		backState.reference		= pDesc->DepthStencil.Back.Reference;

		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType					= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable		= pDesc->DepthStencil.DepthTestEnable;
		depthStencil.depthWriteEnable		= pDesc->DepthStencil.DepthWriteEnable;
		depthStencil.depthCompareOp			= ConvertCompareOpVK(pDesc->DepthStencil.DepthCompareOp);
		depthStencil.depthBoundsTestEnable	= pDesc->DepthStencil.DepthBoundsTestEnable;
		depthStencil.stencilTestEnable		= pDesc->DepthStencil.StencilTestEnable;
		depthStencil.front					= frontState;
		depthStencil.back					= backState;
		depthStencil.minDepthBounds			= pDesc->DepthStencil.MinDepthBounds;
		depthStencil.maxDepthBounds			= pDesc->DepthStencil.MaxDepthBounds;

		// Color blend
		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
		colorBlendAttachments.reserve(pDesc->ColorBlendState.ColorBlendAttachments.size());
		for (auto& attachment : pDesc->ColorBlendState.ColorBlendAttachments)
		{
			VkPipelineColorBlendAttachmentState attachmentVK = {};
			attachmentVK.colorWriteMask			= ConvertColorComponentVK(attachment.ColorWriteMask);
			attachmentVK.blendEnable			= attachment.BlendEnable;
			attachmentVK.srcColorBlendFactor	= ConvertBlendFactorVK(attachment.SrcColorBlendFactor);
			attachmentVK.dstColorBlendFactor	= ConvertBlendFactorVK(attachment.DstColorBlendFactor);
			attachmentVK.colorBlendOp			= ConvertBlendOpVK(attachment.ColorBlendOp);
			attachmentVK.srcAlphaBlendFactor	= ConvertBlendFactorVK(attachment.SrcAlphaBlendFactor);
			attachmentVK.dstAlphaBlendFactor	= ConvertBlendFactorVK(attachment.DstAlphaBlendFactor);
			attachmentVK.alphaBlendOp			= ConvertBlendOpVK(attachment.AlphaBlendOp);
			colorBlendAttachments.push_back(attachmentVK);
		}

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType				= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable		= pDesc->ColorBlendState.LogicOpEnable;
		colorBlending.logicOp			= ConvertLogicOpVK(pDesc->ColorBlendState.LogicOp);
		colorBlending.attachmentCount	= static_cast<uint32>(colorBlendAttachments.size());
		colorBlending.pAttachments		= colorBlendAttachments.data();
		colorBlending.blendConstants[0]	= pDesc->ColorBlendState.BlendConstants[0];
		colorBlending.blendConstants[1]	= pDesc->ColorBlendState.BlendConstants[1];
		colorBlending.blendConstants[2]	= pDesc->ColorBlendState.BlendConstants[2];
		colorBlending.blendConstants[3]	= pDesc->ColorBlendState.BlendConstants[3];

		// Dynamic state
		std::vector<VkDynamicState> dynamicStates;
		if (pDesc->Viewport.IsDynamic) dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		if (pDesc->Scissor.IsDynamic) dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
		dynamicStates.shrink_to_fit();

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType				= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount	= static_cast<uint32>(dynamicStates.size());
		dynamicState.pDynamicStates		= dynamicStates.data();

		// Create shader stage infos
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		shaderStages.reserve(
			(uint32)(pDesc->pVertexShader != nullptr) +
			(uint32)(pDesc->pFragmentShader != nullptr));

		if (pDesc->pVertexShader)
		{
			PVKShader* pVertexShader = reinterpret_cast<PVKShader*>(pDesc->pVertexShader);
			POLY_VALIDATE(pVertexShader->GetShaderStage() == FShaderStage::VERTEX, "Shader type of Vertex does not match!");
			shaderStages.push_back(pVertexShader->GetPipelineInfo());
		}

		if (pDesc->pFragmentShader)
		{
			PVKShader* pFragmentShader = reinterpret_cast<PVKShader*>(pDesc->pFragmentShader);
			POLY_VALIDATE(pFragmentShader->GetShaderStage() == FShaderStage::FRAGMENT, "Shader type of Fragment does not match!");
			shaderStages.push_back(pFragmentShader->GetPipelineInfo());
		}

		if (!pDesc->pVertexShader && !pDesc->pFragmentShader)
			POLY_VALIDATE(false, "Atleast one shader must be bound to a graphics pipeline!");

		// Finally, create the pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType					= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount				= static_cast<uint32>(shaderStages.size());
		pipelineInfo.pStages				= shaderStages.data();
		pipelineInfo.pVertexInputState		= &vertexInputInfo;
		pipelineInfo.pInputAssemblyState	= &inputAssembly;
		pipelineInfo.pViewportState			= &viewportState;
		pipelineInfo.pRasterizationState	= &rasterizer;
		pipelineInfo.pMultisampleState		= &multisampling;
		pipelineInfo.pDepthStencilState		= &depthStencil;
		pipelineInfo.pColorBlendState		= &colorBlending;
		pipelineInfo.pDynamicState			= &dynamicState;
		pipelineInfo.layout					= m_pPipelineLayout->GetNativeVK();
		pipelineInfo.renderPass				= reinterpret_cast<PVKRenderPass*>(pDesc->pRenderPass)->GetNativeVK();
		pipelineInfo.subpass				= pDesc->Subpass;
		pipelineInfo.basePipelineHandle		= VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex		= -1; // Optional

		PVK_CHECK(vkCreateGraphicsPipelines(PVKInstance::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline), "Failed to create graphics pipeline!");
	}
}