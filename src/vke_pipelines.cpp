#include "vke_pipelines.hpp"
#include "vke_device.hpp"

namespace vke {

VkeGraphicsPipeline VkeGraphicsPipeline::init() {
	m_shaderStages.clear();
	m_pipelineLayout = {};
	m_colorBlendAttachment = {};

	m_inputAssembly = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};

	m_rasterizer = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};

	m_multisampling = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};

	m_depthStencil = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

	m_renderInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};

	return *this;
}

VkGraphicsPipelineCreateInfo VkeGraphicsPipeline::buildPipelineInfo() {
	VkPipelineViewportStateCreateInfo viewportState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.viewportCount = 1,
		.scissorCount = 1,
	};

	VkPipelineColorBlendStateCreateInfo colorBlending = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext = nullptr,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &m_colorBlendAttachment,
	};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

	VkDynamicState state[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = &state[0],
	};

	VkGraphicsPipelineCreateInfo pipelineInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &m_renderInfo,
		.stageCount = (uint32_t)m_shaderStages.size(),
		.pStages = m_shaderStages.data(),
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &m_inputAssembly,
		.pViewportState = &viewportState,
		.pRasterizationState = &m_rasterizer,
		.pMultisampleState = &m_multisampling,
		.pDepthStencilState = &m_depthStencil,
		.pColorBlendState = &colorBlending,
		.pDynamicState = &dynamicState,
		.layout = m_pipelineLayout,
	};

	return pipelineInfo;
}

VkeGraphicsPipeline VkeGraphicsPipeline::setPipelineLayout(VkPipelineLayout layout) {
	m_pipelineLayout = layout;
	return *this;
}

VkeGraphicsPipeline VkeGraphicsPipeline::setColorAttachmentFormat(VkFormat format) {
	m_colorAttachmentFormat = format;
	m_renderInfo.colorAttachmentCount = 1;
	m_renderInfo.pColorAttachmentFormats = &m_colorAttachmentFormat;
	return *this;
}

VkeGraphicsPipeline VkeGraphicsPipeline::setDepthFormat(VkFormat format) {
	m_renderInfo.depthAttachmentFormat = format;
	return *this;
}

VkeGraphicsPipeline VkeGraphicsPipeline::disableDepthTest() {
	m_depthStencil.depthTestEnable = VK_FALSE;
	m_depthStencil.depthWriteEnable = VK_FALSE;
	m_depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
	m_depthStencil.depthBoundsTestEnable = VK_FALSE;
	m_depthStencil.stencilTestEnable = VK_FALSE;
	m_depthStencil.front = {};
	m_depthStencil.back = {};
	m_depthStencil.minDepthBounds = 0.f;
	m_depthStencil.maxDepthBounds = 1.f;
	m_renderInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;

	return *this;
}

} // namespace vke
