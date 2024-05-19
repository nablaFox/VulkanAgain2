#include "vke_pipelines.hpp"
#include "vke_device.hpp"
#include "vke_initializers.hpp"

namespace vke {

VkePipeline::VkePipeline() {
	m_pipeline = VK_NULL_HANDLE;
	m_pipelineLayout = {};
}

void VkePipeline::pushConstants(VkCommandBuffer cmd, GPUDrawPushConstants* constants, VkShaderStageFlags stage) {
	vkCmdPushConstants(cmd, m_pipelineLayout, stage, 0, sizeof(GPUDrawPushConstants), constants);
}

VkePipeline& VkePipeline::setDescriptorSet(VkeDescriptorSet& descriptorSet) {
	m_descriptorLayouts.push_back(&descriptorSet.m_descriptorSetLayout);
	m_descriptorSets.push_back(&descriptorSet.m_descriptorSet);

	m_pipelineLayoutInfo.setLayoutCount = (uint32_t)m_descriptorLayouts.size();
	m_pipelineLayoutInfo.pSetLayouts = *(m_descriptorLayouts.data());

	return *this;
}

// Graphics Pipeline
VkeGraphicsPipeline::VkeGraphicsPipeline() {
	m_shaderStages.clear();
	m_colorBlendAttachment = {};
	m_pipelineLayoutInfo = vkinit::graphicsPipelineLayoutCreateInfo();

	m_inputAssembly = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};

	m_rasterizer = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};

	m_multisampling = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};

	m_depthStencil = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};

	m_renderInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
}

void VkeGraphicsPipeline::bind(VkCommandBuffer cmd) {
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

	if (!m_descriptorSets.empty())
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, m_descriptorSets.size(),
								*m_descriptorSets.data(), 0, nullptr);
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

VkeGraphicsPipeline& VkeGraphicsPipeline::setPushConstantRange(VkPushConstantRange& bufferRange, uint32_t count) {
	m_pipelineLayoutInfo.pushConstantRangeCount = count;
	m_pipelineLayoutInfo.pPushConstantRanges = &bufferRange;
	return *this;
}

// Compute Pipeline
VkeComputePipeline::VkeComputePipeline() {
	m_computeInfo = {.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
	m_pipelineLayoutInfo = vkinit::computePipelineLayoutCreateInfo();
}

void VkeComputePipeline::bind(VkCommandBuffer cmd) {
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline);

	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, m_descriptorSets.size(),
							*m_descriptorSets.data(), 0, nullptr);
}

VkeComputePipeline& VkeComputePipeline::setShader(VkeShader& computeShader) {
	m_computeInfo.stage = vkinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_COMPUTE_BIT, computeShader.getModule());
	return *this;
}

} // namespace vke

VkPushConstantRange vkutil::getPushConstantRange(VkShaderStageFlags stage, uint32_t size, uint32_t offset) {
	return {
		stage,
		offset,
		size,
	};
}
