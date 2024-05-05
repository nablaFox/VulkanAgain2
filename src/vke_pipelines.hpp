#pragma once

#include "vke_shader.hpp"
#include "vke_swapchain.hpp"
#include "vke_types.hpp"

namespace vke {

class VkePipeline {

protected:
	VkPipeline m_pipeline;
	VkPipelineLayout m_pipelineLayout;
};

class VkeGraphicsPipeline : public VkePipeline {

	friend class VkeDevice;

public:
	VkeGraphicsPipeline() {}

	VkeGraphicsPipeline init();
	VkeGraphicsPipeline setPipelineLayout(VkPipelineLayout layout);
	VkeGraphicsPipeline setShaders(VkeShader& vertexShader, VkeShader& fragmentShader);
	VkeGraphicsPipeline setInputTopology(VkPrimitiveTopology topology);
	VkeGraphicsPipeline setPolygonMode(VkPolygonMode mode);
	VkeGraphicsPipeline setCullMode(VkCullModeFlags mode, VkFrontFace frontFace);
	VkeGraphicsPipeline setMultisamplingNone();
	VkeGraphicsPipeline disableBlending();
	VkeGraphicsPipeline setColorAttachmentFormat(VkFormat format);
	VkeGraphicsPipeline setDepthFormat(VkFormat format);
	VkeGraphicsPipeline enableDepthTest(bool enable, VkCompareOp op = VK_COMPARE_OP_LESS);
	VkeGraphicsPipeline disableDepthTest();
	VkeGraphicsPipeline enableBlendingAdditive();
	VkeGraphicsPipeline enableBlendingAlphablend();

private:
	VkGraphicsPipelineCreateInfo buildPipelineInfo();

	std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;
	VkPipelineColorBlendAttachmentState m_colorBlendAttachment;
	VkFormat m_colorAttachmentFormat;

	VkPipelineInputAssemblyStateCreateInfo m_inputAssembly;
	VkPipelineRasterizationStateCreateInfo m_rasterizer;
	VkPipelineMultisampleStateCreateInfo m_multisampling;
	VkPipelineDepthStencilStateCreateInfo m_depthStencil;
	VkPipelineRenderingCreateInfo m_renderInfo;
};

class VkeComputePipeline : public VkePipeline {};

} // namespace vke
