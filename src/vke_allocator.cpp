#include "vke_allocator.hpp"

#include "vke_device.hpp"
#include "vke_initializers.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace vke {

void VkeAllocator::init() {
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = m_device->m_chosenGPU;
	allocatorInfo.device = m_device->m_device;
	allocatorInfo.instance = m_device->m_instance;
	allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	vmaCreateAllocator(&allocatorInfo, &m_allocator);
}

void VkeAllocator::createDrawImage(VkExtent2D extent, AllocatedImage* image) {
	VkExtent3D drawImageExtent = {
		extent.width,
		extent.height,
		1,
	};

	image->imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	image->imageExtent = extent;

	VkImageUsageFlags drawImageUsages{};
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	auto imageCreateInfo = vkinit::imageCreateInfo(image->imageFormat, drawImageUsages, drawImageExtent);

	VmaAllocationCreateInfo imageAllocInfo{};
	imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	imageAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vmaCreateImage(m_allocator, &imageCreateInfo, &imageAllocInfo, &image->image, &image->allocation, nullptr);

	auto imageViewCreateInfo = vkinit::imageViewCreateInfo(image->imageFormat, image->image, VK_IMAGE_ASPECT_COLOR_BIT);

	VK_CHECK(vkCreateImageView(m_device->m_device, &imageViewCreateInfo, nullptr, &image->imageView));
}

void VkeAllocator::destroyImage(AllocatedImage& image) {
	vkDestroyImageView(m_device->m_device, image.imageView, nullptr);
	vmaDestroyImage(m_allocator, image.image, image.allocation);
}

VkeAllocator::~VkeAllocator() { assert(m_device == nullptr); }

void VkeAllocator::destroy() {
	vmaDestroyAllocator(m_allocator);
	m_device = nullptr;
}

} // namespace vke
