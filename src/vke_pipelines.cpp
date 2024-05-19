#include "vke_pipelines.hpp"
#include "vke_device.hpp"
#include "vke_initializers.hpp"

namespace vke {

VkeGraphicsPipeline::VkeGraphicsPipeline() {
	m_shaderStages.clear();
	m_pipelineLayout = {};
	m_colorBlendAttachment = {};

	m_inputAssembly = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};

	m_rasterizer = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};

	m_multisampling = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};

	m_depthStencil = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

	m_renderInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
}

VkGraphicsPipelineCreateInfo VkeGraphicsPipeline::buildPipelineInfo() {
	auto viewportState = new VkPipelineViewportStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.viewportCount = 1,
		.scissorCount = 1,
	};

	auto colorBlending = new VkPipelineColorBlendStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext = nullptr,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &m_colorBlendAttachment,
	};

	auto vertexInputInfo =
		new VkPipelineVertexInputStateCreateInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

	VkDynamicState* state = new VkDynamicState[2];
	state[0] = VK_DYNAMIC_STATE_VIEWPORT;
	state[1] = VK_DYNAMIC_STATE_SCISSOR;

	auto dynamicState = new VkPipelineDynamicStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = &state[0],
	};

	VkGraphicsPipelineCreateInfo pipelineInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &m_renderInfo,
		.stageCount = (uint32_t)m_shaderStages.size(),
		.pStages = m_shaderStages.data(),
		.pVertexInputState = vertexInputInfo,
		.pInputAssemblyState = &m_inputAssembly,
		.pViewportState = viewportState,
		.pRasterizationState = &m_rasterizer,
		.pMultisampleState = &m_multisampling,
		.pDepthStencilState = &m_depthStencil,
		.pColorBlendState = colorBlending,
		.pDynamicState = dynamicState,
		.layout = m_pipelineLayout,
	};

	return pipelineInfo;
}

void VkeGraphicsPipeline::bind(VkCommandBuffer cmd) { vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline); }

void VkeGraphicsPipeline::pushConstants(VkCommandBuffer cmd, GPUDrawPushConstants* constants) {
	vkCmdPushConstants(cmd, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), constants);
}

VkeGraphicsPipeline& VkeGraphicsPipeline::setShaders(VkeShader& vertexShader, VkeShader& fragmentShader) {
	m_shaderStages.clear();
	m_shaderStages.push_back(vkinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertexShader.getModule()));
	m_shaderStages.push_back(vkinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader.getModule()));
	return *this;
}

VkeGraphicsPipeline& VkeGraphicsPipeline::setInputTopology(VkPrimitiveTopology topology) {
	m_inputAssembly.topology = topology;
	m_inputAssembly.primitiveRestartEnable = VK_FALSE;
	return *this;
}

VkeGraphicsPipeline& VkeGraphicsPipeline::setPolygonMode(VkPolygonMode mode) {
	m_rasterizer.polygonMode = mode;
	m_rasterizer.lineWidth = 1.0f;
	return *this;
}

VkeGraphicsPipeline& VkeGraphicsPipeline::setCullMode(VkCullModeFlags mode, VkFrontFace frontFace) {
	m_rasterizer.cullMode = mode;
	m_rasterizer.frontFace = frontFace;
	return *this;
}

VkeGraphicsPipeline& VkeGraphicsPipeline::setMultisamplingNone() {
	m_multisampling.sampleShadingEnable = VK_FALSE;
	m_multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	m_multisampling.minSampleShading = 1.0f;
	m_multisampling.pSampleMask = nullptr;
	m_multisampling.alphaToCoverageEnable = VK_FALSE;
	m_multisampling.alphaToOneEnable = VK_FALSE;
	return *this;
}

VkeGraphicsPipeline& VkeGraphicsPipeline::disableBlending() {
	m_colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	m_colorBlendAttachment.blendEnable = VK_FALSE;
	return *this;
}

VkeGraphicsPipeline& VkeGraphicsPipeline::setColorAttachmentFormat(VkFormat format) {
	m_colorAttachmentFormat = format;
	m_renderInfo.colorAttachmentCount = 1;
	m_renderInfo.pColorAttachmentFormats = &m_colorAttachmentFormat;
	return *this;
}

VkeGraphicsPipeline& VkeGraphicsPipeline::setDepthFormat(VkFormat format) {
	m_renderInfo.depthAttachmentFormat = format;
	return *this;
}

VkeGraphicsPipeline& VkeGraphicsPipeline::enableDepthTest(bool enable, VkCompareOp op) { return *this; }

VkeGraphicsPipeline& VkeGraphicsPipeline::disableDepthTest() {
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

VkeGraphicsPipeline& VkeGraphicsPipeline::enableBlendingAdditive() { return *this; }

VkeGraphicsPipeline& VkeGraphicsPipeline::enableBlendingAlphablend() { return *this; }

} // namespace vke
