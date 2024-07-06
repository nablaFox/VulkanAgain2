#pragma once

#include "vke_types.hpp"

namespace vke {

class VkeDescriptor {
	friend class VkePipeline;
	friend struct VkeDescriptorAllocator;
	friend class VkeDevice;

public:
	void addBinding(uint32_t binding, VkDescriptorType type);
	void writeImage(uint32_t binding, VkImageView imageView, VkSampler sampler, VkImageLayout layout);
	void writeBuffer(uint32_t binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type);

private:
	VkResult initLayout(VkDevice device, VkShaderStageFlags shaderStages);

	VkDescriptorSet m_descriptorSet;
	VkDescriptorSetLayout m_descriptorSetLayout;

	std::vector<VkDescriptorSetLayoutBinding> m_bindings;
	std::vector<VkDescriptorImageInfo> m_imageInfos;
	std::vector<VkDescriptorBufferInfo> m_bufferInfos;
	std::vector<VkWriteDescriptorSet> m_writes;
};

struct VkeDescriptorAllocator {
	struct PoolSizeRatio {
		VkDescriptorType type;
		float ratio;
	};

	VkDescriptorPool m_pool;
	VkResult initPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
	VkResult resetDescriptorPool(VkDevice device);
	VkResult allocate(VkDevice device, VkeDescriptor* descriptorSet);
	void destroyPoll(VkDevice device);
};

} // namespace vke
