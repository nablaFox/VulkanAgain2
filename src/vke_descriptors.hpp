#pragma once

#include "vke_types.hpp"

namespace vke {

class VkeDescriptorSet {
	friend class VkePipeline;
	friend struct DescriptorAllocator;
	friend class VkeDevice;

public:
	void bindImage(VkImageView imageView, VkSampler sampler, VkImageLayout layout, VkShaderStageFlags stage,
				   uint32_t binding = 0);
	void bindBuffer();

private:
	VkDescriptorSet m_descriptorSet;
	VkDescriptorSetLayout m_descriptorSetLayout;

	std::vector<VkDescriptorSetLayoutBinding> m_bindings;
	std::vector<VkDescriptorImageInfo> m_imageInfos;
	std::vector<VkWriteDescriptorSet> m_writes;
};

struct DescriptorAllocator {
	struct PoolSizeRatio {
		VkDescriptorType type;
		float ratio;
	};

	VkDescriptorPool m_pool;

	void initPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
	void clearDescriptors(VkDevice device);
	void destroyPoll(VkDevice device);

	VkResult allocate(VkDevice device, VkeDescriptorSet* descriptorSet);
};

} // namespace vke
