#include "vke_engine.hpp"
#include "vke_device.hpp"
#include "vke_initializers.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace vke {

constexpr bool useValidationLayers = true; // TODO: handle debug mode in a better way

VkResult VkeDevice::init(VkeWindow* window) {
	vkb::InstanceBuilder builder;

	auto instRet = builder.set_app_name("Vulkan Engine")
					   .request_validation_layers(useValidationLayers)
					   .use_default_debug_messenger()
					   .require_api_version(1, 3, 0)
					   .build();

	vkb::Instance vkbInst = instRet.value();

	m_vkInstance = vkbInst.instance;
	m_debugMessenger = vkbInst.debug_messenger;

	// physical and logical devices
	VK_RETURN(glfwCreateWindowSurface(m_vkInstance, window->getWindow(), nullptr, &m_surface));

	VkPhysicalDeviceVulkan13Features features{};
	features.dynamicRendering = true;
	features.synchronization2 = true;

	VkPhysicalDeviceVulkan12Features features12{};
	features12.bufferDeviceAddress = true;
	features12.descriptorIndexing = true;

	vkb::PhysicalDeviceSelector selector{vkbInst};
	vkb::PhysicalDevice phyisicalDevice = selector.set_minimum_version(1, 3)
											  .set_required_features_13(features)
											  .set_required_features_12(features12)
											  .set_surface(m_surface)
											  .select()
											  .value();

	vkb::DeviceBuilder deviceBuilder{phyisicalDevice};
	vkb::Device vkbDevice = deviceBuilder.build().value();

	m_chosenGPU = vkbDevice.physical_device;
	m_device = vkbDevice.device;

	m_graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	m_graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

	VmaAllocatorCreateInfo allocatorInfo = {
		.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
		.physicalDevice = m_chosenGPU,
		.device = m_device,
		.instance = m_vkInstance,
	};

	VK_RETURN(vmaCreateAllocator(&allocatorInfo, &m_allocator));

	m_deletionQueue.push_function([this] { vmaDestroyAllocator(m_allocator); });

	return VK_SUCCESS;
}

VkResult VkeDevice::createCommandPool(VkCommandPool* pool, VkCommandPoolCreateFlags flags) {
	auto commandPoolCreateInfo = vkinit::commandPoolCreateInfo(m_graphicsQueueFamily, flags);

	m_deletionQueue.push_function([this, pool] { vkDestroyCommandPool(m_device, *pool, nullptr); });

	return vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, pool);
}

VkResult VkeDevice::allocateCommandBuffer(VkCommandBuffer* buffer, VkCommandPool pool) {
	auto cmdAllocInfo = vkinit::commandBufferAllocateInfo(pool);
	return vkAllocateCommandBuffers(m_device, &cmdAllocInfo, buffer);
}

VkResult VkeDevice::createSemaphore(VkSemaphore* semaphore, VkSemaphoreCreateFlags flags) {
	auto semaphoreCreateInfo = vkinit::semaphoreCreateInfo(flags);

	m_deletionQueue.push_function([this, semaphore] { vkDestroySemaphore(m_device, *semaphore, nullptr); });

	return vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, semaphore);
}

VkResult VkeDevice::createFence(VkFence* fence, VkFenceCreateFlags flags) {
	auto fenceCreateInfo = vkinit::fenceCreateInfo(flags);

	m_deletionQueue.push_function([this, fence] { vkDestroyFence(m_device, *fence, nullptr); });

	return vkCreateFence(m_device, &fenceCreateInfo, nullptr, fence);
}

VkResult VkeDevice::initFrameData(FrameData* frame, int count) {
	for (int i = 0; i < count; i++) {
		VK_RETURN(createCommandPool(&frame[i]._commandPool, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
		VK_RETURN(allocateCommandBuffer(&frame[i]._commandBuffer, frame[i]._commandPool));

		VK_RETURN(createSemaphore(&frame[i]._swapchainSemaphore));
		VK_RETURN(createSemaphore(&frame[i]._renderSemaphore));
		VK_RETURN(createFence(&frame[i]._renderFence, VK_FENCE_CREATE_SIGNALED_BIT));
	}

	return VK_SUCCESS;
}

VkResult VkeDevice::createShader(VkeShader& shader, const char* path) {
	std::vector<uint32_t> buffer;
	VK_RETURN(shader.loadShaderModule(path, buffer));

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.codeSize = buffer.size() * sizeof(uint32_t);
	createInfo.pCode = buffer.data();

	VK_RETURN(vkCreateShaderModule(m_device, &createInfo, nullptr, &shader.m_shaderModule));

	fmt::println("Shader module created: {}", path);

	return VK_SUCCESS;
}

VkResult VkeDevice::destroyShader(VkeShader& shader) {
	vkDestroyShaderModule(m_device, shader.m_shaderModule, nullptr);
	return VK_SUCCESS;
}

VkResult VkeDevice::createPipelineLayout(VkePipeline& pipeline, VkPipelineLayoutCreateInfo& layoutInfo) {
	VK_RETURN(vkCreatePipelineLayout(m_device, &layoutInfo, nullptr, &pipeline.m_pipelineLayout));
	return VK_SUCCESS;
}

VkResult VkeDevice::createGraphicsPipeline(VkeGraphicsPipeline& pipeline) {
	VkGraphicsPipelineCreateInfo pipelineInfo = pipeline.buildPipelineInfo();
	VK_RETURN(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.m_pipeline));

	m_deletionQueue.push_function([this, &pipeline] {
		vkDestroyPipeline(m_device, pipeline.m_pipeline, nullptr);
		vkDestroyPipelineLayout(m_device, pipeline.m_pipelineLayout, nullptr);
	});

	return VK_SUCCESS;
}

VkResult VkeDevice::createDrawImage(VkExtent2D extent, AllocatedImage* image) {
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

	VK_RETURN(vkCreateImageView(m_device, &imageViewCreateInfo, nullptr, &image->imageView));

	m_deletionQueue.push_function([this, image] {
		vkDestroyImageView(m_device, image->imageView, nullptr);
		vmaDestroyImage(m_allocator, image->image, image->allocation);
	});

	return VK_SUCCESS;
}

VkResult VkeDevice::submitCommand(int submitCount, VkSubmitInfo2* submitInfo, VkFence fence) {
	VK_RETURN(vkQueueSubmit2(m_graphicsQueue, submitCount, submitInfo, fence));
	return VK_SUCCESS;
}

VkResult VkeDevice::createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, AllocatedBuffer* buffer,
								 bool temp) {
	VkBufferCreateInfo bufferInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = nullptr,
		.size = allocSize,
		.usage = usage,
	};

	VmaAllocationCreateInfo vmaallocInfo = {
		.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
		.usage = memoryUsage,
	};

	VK_RETURN(vmaCreateBuffer(m_allocator, &bufferInfo, &vmaallocInfo, &buffer->buffer, &buffer->allocation, &buffer->info));

	if (!temp)
		m_deletionQueue.push_function([this, &buffer] { destroyBuffer(buffer); });

	return VK_SUCCESS;
}

VkResult VkeDevice::createStagingBuffer(size_t allocSize, AllocatedBuffer* staging, void*& data) {
	VK_RETURN(createBuffer(allocSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, staging, true));
	data = staging->allocation->GetMappedData();
	return VK_SUCCESS;
}

VkResult VkeDevice::destroyBuffer(AllocatedBuffer* buffer) {
	vmaDestroyBuffer(m_allocator, buffer->buffer, buffer->allocation);
	return VK_SUCCESS;
}

void VkeDevice::destroy() {
	m_deletionQueue.flush();

	vkDestroySurfaceKHR(m_vkInstance, m_surface, nullptr);

	vkDestroyDevice(m_device, nullptr);
	vkb::destroy_debug_utils_messenger(m_vkInstance, m_debugMessenger);
	vkDestroyInstance(m_vkInstance, nullptr);
}

} // namespace vke
