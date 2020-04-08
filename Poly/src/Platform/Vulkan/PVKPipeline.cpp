#include "polypch.h"
#include "PVKPipeline.h"
#include "PVKInstance.h"
#include "PVKSwapChain.h"
#include "VulkanCommon.h"
#include "PVKShader.h"
#include "PVKRenderPass.h"
#include "PVKDescriptor.h"

// TODO: Make the pipeline support both compute and graphcis pipelines
// TODO: Make it possible to edit different aspects of the pipeline (rasterizer, multisample, etc.) before init

namespace Poly
{

	PVKPipeline::PVKPipeline() :
		swapChain(nullptr),
		pipeline(VK_NULL_HANDLE), pipelineLayout(VK_NULL_HANDLE),
		renderPass(nullptr), shader(nullptr), pipelineType(VK_PIPELINE_BIND_POINT_GRAPHICS), descriptor(nullptr)
	{
	}

	PVKPipeline::~PVKPipeline()
	{
	}

	void PVKPipeline::init(PVKSwapChain& swapChain, PVKShader& shader, PVKRenderPass& renderPass)
	{
		this->swapChain = &swapChain;
		this->renderPass = &renderPass;

		this->shader = &shader;

		createPipeline();
	}

	void PVKPipeline::cleanup()
	{
		vkDestroyPipeline(PVKInstance::getDevice(), this->pipeline, nullptr);
		vkDestroyPipelineLayout(PVKInstance::getDevice(), this->pipelineLayout, nullptr);
	}

	void PVKPipeline::addVertexDescriptions(uint32_t binding, uint32_t location, uint32_t stride, VkFormat format)
	{
		VkVertexInputBindingDescription bindingDesc = {};
		bindingDesc.binding = binding;
		bindingDesc.stride = stride;
		bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		this->vertexBinding.push_back(bindingDesc);

		VkVertexInputAttributeDescription attribDesc = {};
		attribDesc.binding = binding;
		attribDesc.location = location;
		attribDesc.format = format;
		attribDesc.offset = 0;
		this->vertexAttributes.push_back(attribDesc);
	}

	void PVKPipeline::setDescriptor(PVKDescriptor& descriptor)
	{
		this->descriptor = &descriptor;
	}

	void PVKPipeline::createPipeline()
	{
		VkExtent2D extent = this->swapChain->getExtent();

		// Vertexbuffer attribute info
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(this->vertexBinding.size());
		vertexInputInfo.pVertexBindingDescriptions = this->vertexBinding.data();
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(this->vertexAttributes.size());
		vertexInputInfo.pVertexAttributeDescriptions = this->vertexAttributes.data();

		// Topology info (how to draw the triangles)
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// Viewport and scissors
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)extent.width;
		viewport.height = (float)extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		// Rasterizer (Cull, polygon mode, depthbias)
		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_NONE; // VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		// Multisampling
		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		/* When having depth and stencil testing, add this here! */
		// VkPipelineDepthStencilStateCreateInfo

		// Color blending (if blending is needed, change here)
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		// Dynamic states can be changed without needing to reconstruct the pipeline
		VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
		};

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;

		// Pipeline layout, specifies uniforms and push layouts to the shaders
		auto& setLayouts = this->descriptor->getSetLayouts();
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = setLayouts.size(); // Optional
		pipelineLayoutInfo.pSetLayouts = setLayouts.data(); // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		// Create pipeline layout, the description of all the fixed-function stages
		PVK_CHECK(vkCreatePipelineLayout(PVKInstance::getDevice(), &pipelineLayoutInfo, nullptr, &this->pipelineLayout), "Failed to create pipeline layout!");

		// Pipeline creation info
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		std::vector<VkPipelineShaderStageCreateInfo> infos = this->shader->getShaderCreateInfos();
		pipelineInfo.stageCount = infos.size();
		pipelineInfo.pStages = infos.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr; // Optional
		pipelineInfo.layout = this->pipelineLayout;
		pipelineInfo.renderPass = this->renderPass->getRenderPass();
		pipelineInfo.subpass = 0;
		// When creating new pipelines, change basePipelineHandle OR basePipelineIndex, two different apporaches!
		// These will only work when "VK_PIPELINE_CREATE_DERIVATIVE_BIT" is set in 'flags' of VkGraphicsPiplineCreateInfo
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		// Create the pipeline (function can create multiple pipelines at the same time)
		// The nullptr is a reference to a VkPipelineCache which can speed up creation performance if used
		PVK_CHECK(vkCreateGraphicsPipelines(PVKInstance::getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->pipeline), "Failed to create graphics pipeline!");

		this->pipelineType = VK_PIPELINE_BIND_POINT_GRAPHICS;
	}

}