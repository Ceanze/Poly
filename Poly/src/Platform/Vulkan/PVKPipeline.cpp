#include "polypch.h"
#include "PVKPipeline.h"
#include "PVKInstance.h"
#include "PVKSwapChain.h"
#include "VulkanCommon.h"

namespace Poly
{

	PVKPipeline::PVKPipeline() :
		device(VK_NULL_HANDLE), extent({0, 0}), imageFormat(VK_FORMAT_UNDEFINED),
		pipeline(VK_NULL_HANDLE), pipelineLayout(VK_NULL_HANDLE)
	{
	}

	PVKPipeline::~PVKPipeline()
	{
	}

	void PVKPipeline::init(PVKInstance* instance, PVKSwapChain* swapChain)
	{
		this->device = instance->getDevice();
		this->extent = swapChain->getExtent();
		this->imageFormat = swapChain->getFormat();

		//addShader(ShaderType::Vertex, "\\shaders\\vert.spv");
		//addShader(ShaderType::Fragment, "\\shaders\\frag.spv");
		addShader(ShaderType::Vertex, "C:\\dev\\Poly\\Sandbox\\shaders\\vert.spv");
		addShader(ShaderType::Fragment, "C:\\dev\\Poly\\Sandbox\\shaders\\frag.spv");

		createPipeline();
	}

	void PVKPipeline::cleanup()
	{
		vkDestroyPipeline(this->device, this->pipeline, nullptr);
		vkDestroyPipelineLayout(this->device, this->pipelineLayout, nullptr);
		this->renderPass.cleanup();
		
		for (auto shaderStage : this->shadersStages)
			vkDestroyShaderModule(this->device, shaderStage.module, nullptr);
	}

	void PVKPipeline::addShader(ShaderType shader, const std::string& path)
	{
		std::vector<char> shaderCode = readFile(path);

		createShaderModule(shader, shaderCode);
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

	void PVKPipeline::createShaderModule(ShaderType shader, const std::vector<char>& code)
	{
		// TODO: Refactor shaders to its own class?

		VkShaderStageFlagBits shaderStage = VK_SHADER_STAGE_VERTEX_BIT;
		switch (shader) {
		case ShaderType::Vertex:
			break;
		case ShaderType::Fragment:
			shaderStage = VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		}

		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(this->device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		VkPipelineShaderStageCreateInfo shaderStageInfo = {};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = shaderStage;
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = "main"; // Main should always be considered default

		this->shadersStages.push_back(shaderStageInfo);
	}

	void PVKPipeline::createPipeline()
	{
		// TODO: Abstract certain aspects of the pipeline creation (blending, wireframe, ...)
		this->renderPass.init(this->device, this->imageFormat);

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
		viewport.width = (float)this->extent.width;
		viewport.height = (float)this->extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = this->extent;

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
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
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
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		// Create pipeline layout, the description of all the fixed-function stages
		if (vkCreatePipelineLayout(this->device, &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		// Pipeline creation info
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = static_cast<uint32_t>(this->shadersStages.size());
		pipelineInfo.pStages = this->shadersStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr; // Optional
		pipelineInfo.layout = this->pipelineLayout;
		pipelineInfo.renderPass = this->renderPass.getRenderPass();
		pipelineInfo.subpass = 0;
		// When creating new pipelines, change basePipelineHandle OR basePipelineIndex, two different apporaches!
		// These will only work when "VK_PIPELINE_CREATE_DERIVATIVE_BIT" is set in 'flags' of VkGraphicsPiplineCreateInfo
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		// Create the pipeline (function can create multiple pipelines at the same time)
		// The nullptr is a reference to a VkPipelineCache which can speed up creation performance if used
		if (vkCreateGraphicsPipelines(this->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->pipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}
	}

}