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

		// Create vertex arrays
		std::vector<VkVertexInputBindingDescription> vertexBinds;
		std::vector<VkVertexInputAttributeDescription> vertexAttribs;
		vertexBinds.reserve(pDesc->VertexInputs.size());
		vertexAttribs.reserve(pDesc->VertexInputs.size());

		for (auto& vertexInput : pDesc->VertexInputs)
		{
			VkVertexInputBindingDescription bindDesc = {};
			bindDesc.binding	= vertexInput.Binding;
			bindDesc.stride		= vertexInput.Stride;
			bindDesc.inputRate	= ConvertVertexInputRateVK(vertexInput.VertexInputRate);

			VkVertexInputAttributeDescription attribDesc = {};
			attribDesc.binding	= vertexInput.Binding;
			attribDesc.location	= vertexInput.Location;
			attribDesc.format	= ConvertFormatVK(vertexInput.Format);
			attribDesc.offset	= vertexInput.Offset;

			vertexBinds.push_back(bindDesc);
			vertexAttribs.push_back(attribDesc);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.pVertexBindingDescriptions		= vertexBinds.data();
		vertexInputInfo.vertexBindingDescriptionCount	= vertexBinds.size();
		vertexInputInfo.pVertexAttributeDescriptions	= vertexAttribs.data();
		vertexInputInfo.vertexAttributeDescriptionCount	= vertexAttribs.size();
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
		scissor.offset = { pDesc->Scissor.Width, pDesc->Scissor.Height };
		scissor.extent = { pDesc->Scissor.OffsetX, pDesc->Scissor.OffsetY };

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
		frontState.failOp		= ConvertStencilOpVK(pDesc->DepthStencil.Back.FailOp);
		frontState.passOp		= ConvertStencilOpVK(pDesc->DepthStencil.Back.PassOp);
		frontState.depthFailOp	= ConvertStencilOpVK(pDesc->DepthStencil.Back.DepthFailOp);
		frontState.compareOp	= ConvertCompareOpVK(pDesc->DepthStencil.Back.CompareOp);
		frontState.compareMask	= pDesc->DepthStencil.Back.CompareMask;
		frontState.writeMask	= pDesc->DepthStencil.Back.WriteMask;
		frontState.reference	= pDesc->DepthStencil.Back.Reference;

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
		colorBlending.attachmentCount	= colorBlendAttachments.size();
		colorBlending.pAttachments		= colorBlendAttachments.data();
		colorBlending.blendConstants[0]	= pDesc->ColorBlendState.BlendConstants[0];
		colorBlending.blendConstants[1]	= pDesc->ColorBlendState.BlendConstants[1];
		colorBlending.blendConstants[2]	= pDesc->ColorBlendState.BlendConstants[2];
		colorBlending.blendConstants[3]	= pDesc->ColorBlendState.BlendConstants[3];

		// Dynamic state
		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
		};

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType				= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount	= dynamicStates.size();
		dynamicState.pDynamicStates		= dynamicStates.data();

		// Create shader stage infos
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		shaderStages.reserve(
			(uint32)(pDesc->pVertexShader != nullptr) +
			(uint32)(pDesc->pFragmentShader != nullptr));

		if (pDesc->pVertexShader)
		{
			PVKShader* pVertexShader = reinterpret_cast<PVKShader*>(pDesc->pVertexShader);
			POLY_ASSERT(pVertexShader->GetShaderStage() == FShaderStage::VERTEX, "Shader type of Vertex does not match!");
			shaderStages.push_back(pVertexShader->GetPipelineInfo());
		}

		if (pDesc->pFragmentShader)
		{
			PVKShader* pFragmentShader = reinterpret_cast<PVKShader*>(pDesc->pFragmentShader);
			POLY_ASSERT(pFragmentShader->GetShaderStage() == FShaderStage::FRAGMENT, "Shader type of Fragment does not match!");
			shaderStages.push_back(pFragmentShader->GetPipelineInfo());
		}

		// Finally, create the pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType					= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount				= shaderStages.size();
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


	// void PVKPipeline::CreatePipeline()
	// {
	// 	VkExtent2D extent = m_pSwapChain->GetExtent();

	// 	// Vertexbuffer attribute info
	// 	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	// 	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	// 	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(m_VertexBinding.size());
	// 	vertexInputInfo.pVertexBindingDescriptions = m_VertexBinding.data();
	// 	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_VertexAttributes.size());
	// 	vertexInputInfo.pVertexAttributeDescriptions = m_VertexAttributes.data();

	// 	// Topology info (how to draw the triangles)
	// 	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	// 	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	// 	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	// 	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// 	// Viewport and scissors
	// 	VkViewport viewport = {};
	// 	viewport.x = 0.0f;
	// 	viewport.y = 0.0f;
	// 	viewport.width = (float)extent.width;
	// 	viewport.height = (float)extent.height;
	// 	viewport.minDepth = 0.0f;
	// 	viewport.maxDepth = 1.0f;

	// 	VkRect2D scissor = {};
	// 	scissor.offset = { 0, 0 };
	// 	scissor.extent = extent;

	// 	VkPipelineViewportStateCreateInfo viewportState = {};
	// 	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	// 	viewportState.viewportCount = 1;
	// 	viewportState.pViewports = &viewport;
	// 	viewportState.scissorCount = 1;
	// 	viewportState.pScissors = &scissor;

	// 	// Rasterizer (Cull, polygon mode, depthbias)
	// 	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	// 	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	// 	rasterizer.depthClampEnable = VK_FALSE;
	// 	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	// 	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	// 	rasterizer.lineWidth = 1.0f;
	// 	rasterizer.cullMode = VK_CULL_MODE_NONE; // VK_CULL_MODE_BACK_BIT;
	// 	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	// 	rasterizer.depthBiasEnable = VK_FALSE;
	// 	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	// 	rasterizer.depthBiasClamp = 0.0f; // Optional
	// 	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	// 	// Multisampling
	// 	VkPipelineMultisampleStateCreateInfo multisampling = {};
	// 	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	// 	multisampling.sampleShadingEnable = VK_FALSE;
	// 	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	// 	multisampling.minSampleShading = 1.0f; // Optional
	// 	multisampling.pSampleMask = nullptr; // Optional
	// 	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	// 	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	// 	/* When having depth and stencil testing, add this here! */
	// 	// VkPipelineDepthStencilStateCreateInfo

	// 	// Color blending (if blending is needed, change here)
	// 	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	// 	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	// 	colorBlendAttachment.blendEnable = VK_FALSE;
	// 	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	// 	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	// 	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	// 	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	// 	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	// 	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	// 	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	// 	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	// 	colorBlending.logicOpEnable = VK_FALSE;
	// 	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	// 	colorBlending.attachmentCount = 1;
	// 	colorBlending.pAttachments = &colorBlendAttachment;
	// 	colorBlending.blendConstants[0] = 0.0f; // Optional
	// 	colorBlending.blendConstants[1] = 0.0f; // Optional
	// 	colorBlending.blendConstants[2] = 0.0f; // Optional
	// 	colorBlending.blendConstants[3] = 0.0f; // Optional

	// 	// Dynamic states can be changed without needing to reconstruct the pipeline
	// 	VkDynamicState dynamicStates[] = {
	// 		VK_DYNAMIC_STATE_VIEWPORT,
	// 		VK_DYNAMIC_STATE_LINE_WIDTH
	// 	};

	// 	VkPipelineDynamicStateCreateInfo dynamicState = {};
	// 	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	// 	dynamicState.dynamicStateCount = 2;
	// 	dynamicState.pDynamicStates = dynamicStates;

	// 	// Pipeline layout, specifies uniforms and push layouts to the shaders
	// 	auto setLayouts = m_pDescriptor->GetSetLayouts();
	// 	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	// 	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	// 	pipelineLayoutInfo.setLayoutCount = setLayouts.size(); // Optional
	// 	pipelineLayoutInfo.pSetLayouts = setLayouts.data(); // Optional
	// 	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	// 	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	// 	// Create pipeline layout, the description of all the fixed-function stages
	// 	PVK_CHECK(vkCreatePipelineLayout(PVKInstance::GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout), "Failed to create pipeline layout!");

	// 	// Pipeline creation info
	// 	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	// 	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	// 	std::vector<VkPipelineShaderStageCreateInfo> infos = m_pShader->GetShaderCreateInfos();
	// 	pipelineInfo.stageCount = infos.size();
	// 	pipelineInfo.pStages = infos.data();
	// 	pipelineInfo.pVertexInputState = &vertexInputInfo;
	// 	pipelineInfo.pInputAssemblyState = &inputAssembly;
	// 	pipelineInfo.pViewportState = &viewportState;
	// 	pipelineInfo.pRasterizationState = &rasterizer;
	// 	pipelineInfo.pMultisampleState = &multisampling;
	// 	pipelineInfo.pDepthStencilState = nullptr; // Optional
	// 	pipelineInfo.pColorBlendState = &colorBlending;
	// 	pipelineInfo.pDynamicState = nullptr; // Optional
	// 	pipelineInfo.layout = m_PipelineLayout;
	// 	pipelineInfo.renderPass = m_pRenderPass->GetNative();
	// 	pipelineInfo.subpass = 0;
	// 	// When creating new pipelines, change basePipelineHandle OR basePipelineIndex, two different apporaches!
	// 	// These will only work when "VK_PIPELINE_CREATE_DERIVATIVE_BIT" is set in 'flags' of VkGraphicsPiplineCreateInfo
	// 	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	// 	pipelineInfo.basePipelineIndex = -1; // Optional

	// 	// Create the pipeline (function can create multiple pipelines at the same time)
	// 	// The nullptr is a reference to a VkPipelineCache which can speed up creation performance if used
	// 	PVK_CHECK(vkCreateGraphicsPipelines(PVKInstance::GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline), "Failed to create graphics pipeline!");

	// 	m_PipelineType = VK_PIPELINE_BIND_POINT_GRAPHICS;
	// }

}