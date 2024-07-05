#pragma once

#include "vke_descriptors.hpp"
#include "vke_shader.hpp"
#include "vke_swapchain.hpp"
#include "vke_types.hpp"

namespace vke {

class VkePipeline {
	friend class VkeDevice;

public:
	VkePipeline();

	virtual void bind(VkCommandBuffer cmd) = 0;
	void pushConstants(VkCommandBuffer cmd, GPUDrawPushConstants* constants,
					   VkShaderStageFlags stage = VK_SHADER_STAGE_VERTEX_BIT);

	VkePipeline& setDescriptorSet(VkeDescriptor& descriptorSet);

protected:
	VkPipeline m_pipeline;
	VkPipelineLayout m_pipelineLayout;

	VkPipelineLayoutCreateInfo m_pipelineLayoutInfo;

	std::vector<VkDescriptorSetLayout*> m_descriptorLayouts;
	std::vector<VkDescriptorSet*> m_descriptorSets;
};

class VkeGraphicsPipeline : public VkePipeline {
	friend class VkeDevice;

public:
	VkeGraphicsPipeline();
	void bind(VkCommandBuffer cmd) override;

	VkeGraphicsPipeline& setShaders(VkeShader& vertexShader, VkeShader& fragmentShader);
	VkeGraphicsPipeline& setInputTopology(VkPrimitiveTopology topology);
	VkeGraphicsPipeline& setPolygonMode(VkPolygonMode mode);
	VkeGraphicsPipeline& setCullMode(VkCullModeFlags mode, VkFrontFace frontFace);
	VkeGraphicsPipeline& setMultisamplingNone();
	VkeGraphicsPipeline& disableBlending();
	VkeGraphicsPipeline& setColorAttachmentFormat(VkFormat format);
	VkeGraphicsPipeline& setDepthFormat(VkFormat format);
	VkeGraphicsPipeline& enableDepthTest(bool enable, VkCompareOp op = VK_COMPARE_OP_LESS);
	VkeGraphicsPipeline& disableDepthTest();
	VkeGraphicsPipeline& enableBlendingAdditive();
	VkeGraphicsPipeline& enableBlendingAlphablend();
	VkeGraphicsPipeline& setPushConstantRange(VkPushConstantRange& bufferRange, uint32_t count = 1);

private:
	std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;
	VkPipelineColorBlendAttachmentState m_colorBlendAttachment;
	VkFormat m_colorAttachmentFormat;
	VkPipelineInputAssemblyStateCreateInfo m_inputAssembly;
	VkPipelineRasterizationStateCreateInfo m_rasterizer;
	VkPipelineMultisampleStateCreateInfo m_multisampling;
	VkPipelineDepthStencilStateCreateInfo m_depthStencil;
	VkPipelineRenderingCreateInfo m_renderInfo;
};

class VkeComputePipeline : public VkePipeline {
	friend class VkeDevice;

public:
	VkeComputePipeline();
	void bind(VkCommandBuffer cmd) override;

	VkeComputePipeline& setShader(VkeShader& computeShader);

private:
	VkComputePipelineCreateInfo m_computeInfo;
};

} // namespace vke

namespace vkutil {
VkPushConstantRange getPushConstantRange(VkShaderStageFlags stage, uint32_t size, uint32_t offset = 0);
}
