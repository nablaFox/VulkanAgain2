#include "vke_descriptors.hpp"

using namespace vke;

VkResult VkeDescriptor::initLayout(VkDevice device, VkShaderStageFlags shaderStages) {
	for (VkDescriptorSetLayoutBinding& binding : m_bindings) {
		binding.stageFlags |= shaderStages;
	}

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.bindingCount = (uint32_t)m_bindings.size(),
		.pBindings = m_bindings.data(),
	};

	VK_RETURN(vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &m_descriptorSetLayout));
	return VK_SUCCESS;
}

void VkeDescriptor::addBinding(uint32_t binding, VkDescriptorType type) {
	m_bindings.push_back({.binding = binding, .descriptorType = type, .descriptorCount = 1});
}

void VkeDescriptor::writeImage(uint32_t binding, VkImageView imageView, VkSampler sampler, VkImageLayout layout) {
	VkDescriptorImageInfo& info = m_imageInfos.emplace_back(VkDescriptorImageInfo{
		.sampler = sampler,
		.imageView = imageView,
		.imageLayout = layout,
	});

	m_writes.push_back({
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = nullptr,
		.dstSet = VK_NULL_HANDLE,
		.dstBinding = binding,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		.pImageInfo = &info,
	});
}

void VkeDescriptor::writeBuffer(uint32_t binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type) {
	VkDescriptorBufferInfo& info = m_bufferInfos.emplace_back(VkDescriptorBufferInfo{
		.buffer = buffer,
		.offset = offset,
		.range = size,
	});

	VkWriteDescriptorSet write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = VK_NULL_HANDLE,
		.dstBinding = binding,
		.descriptorCount = 1,
		.descriptorType = type,
		.pBufferInfo = &info,
	};

	m_writes.push_back(write);
}

VkResult VkeDescriptorAllocator::initPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios) {
	std::vector<VkDescriptorPoolSize> poolSizes;

	for (PoolSizeRatio ratio : poolRatios) {
		poolSizes.push_back(VkDescriptorPoolSize{
			.type = ratio.type,
			.descriptorCount = uint32_t(ratio.ratio * maxSets),
		});
	}

	VkDescriptorPoolCreateInfo poolInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		.maxSets = maxSets,
		.poolSizeCount = (uint32_t)poolSizes.size(),
		.pPoolSizes = poolSizes.data(),
	};

	return vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_pool);
}

VkResult VkeDescriptorAllocator::resetDescriptorPool(VkDevice device) { return vkResetDescriptorPool(device, m_pool, 0); }

VkResult VkeDescriptorAllocator::allocate(VkDevice device, VkeDescriptor* descriptorSet) {
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

	descriptorSet->m_writes.clear();
	descriptorSet->m_imageInfos.clear();
	descriptorSet->m_bufferInfos.clear();

	return VK_SUCCESS;
}

void VkeDescriptorAllocator::destroyPoll(VkDevice device) { vkDestroyDescriptorPool(device, m_pool, nullptr); }
