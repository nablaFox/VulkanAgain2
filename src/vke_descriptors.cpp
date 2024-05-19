#include "vke_descriptors.hpp"

namespace vke {

void VkeDescriptorSet::bindImage(VkImageView imageView, VkSampler sampler, VkImageLayout layout, VkShaderStageFlags stage,
								 uint32_t binding) {
	m_bindings.push_back({
		.binding = binding,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		.descriptorCount = 1,
		.stageFlags = stage,
		.pImmutableSamplers = nullptr,
	});

	VkDescriptorImageInfo& info = m_imageInfos.emplace_back(VkDescriptorImageInfo{
		.sampler = sampler,
		.imageView = imageView,
		.imageLayout = layout,
	});

	m_writes.push_back({
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = nullptr,
		.dstSet = VK_NULL_HANDLE,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		.pImageInfo = &info,
	});
}

void DescriptorAllocator::initPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios) {
	std::vector<VkDescriptorPoolSize> poolSizes;

	for (PoolSizeRatio ratio : poolRatios) {
		poolSizes.push_back(VkDescriptorPoolSize{
			.type = ratio.type,
			.descriptorCount = uint32_t(ratio.ratio * maxSets),
		});
	}

	VkDescriptorPoolCreateInfo poolInfo = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
	poolInfo.flags = 0;
	poolInfo.maxSets = maxSets;
	poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();

	vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_pool);
}

void DescriptorAllocator::clearDescriptors(VkDevice device) { vkResetDescriptorPool(device, m_pool, 0); }

void DescriptorAllocator::destroyPoll(VkDevice device) { vkDestroyDescriptorPool(device, m_pool, nullptr); }

VkResult DescriptorAllocator::allocate(VkDevice device, VkeDescriptorSet* descriptorSet) {
	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.bindingCount = (uint32_t)descriptorSet->m_bindings.size(),
		.pBindings = descriptorSet->m_bindings.data(),
	};

	VK_RETURN(vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &descriptorSet->m_descriptorSetLayout));

	VkDescriptorSetAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = nullptr,
		.descriptorPool = m_pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &descriptorSet->m_descriptorSetLayout,
	};

	VK_RETURN(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet->m_descriptorSet));

	for (VkWriteDescriptorSet& write : descriptorSet->m_writes) {
		write.dstSet = descriptorSet->m_descriptorSet;
	}

	vkUpdateDescriptorSets(device, (uint32_t)descriptorSet->m_writes.size(), descriptorSet->m_writes.data(), 0, nullptr);

	return VK_SUCCESS;
}

} // namespace vke
