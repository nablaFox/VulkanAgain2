#include "vke_device.hpp"
#include "vke_initializers.hpp"

namespace vke {

constexpr bool useValidationLayers = true; // TODO: handle debug mode in a better way

void VkeDevice::create(VkeDevice* device) {
	static VkeDevice* instance = nullptr;

	if (instance != nullptr)
		return;

	device = new VkeDevice();
	instance = device;
}

VkResult VkeDevice::initialize(VkeWindow* window) {
	vkb::InstanceBuilder builder;

	auto instRet = builder.set_app_name("Vulkan Engine")
					   .request_validation_layers(useValidationLayers)
					   .use_default_debug_messenger()
					   .require_api_version(1, 3, 0)
					   .build();

	vkb::Instance vkbInst = instRet.value();

	m_instance = vkbInst.instance;
	m_debugMessenger = vkbInst.debug_messenger;

	// physical and logical devices
	VK_RETURN(glfwCreateWindowSurface(m_instance, window->getWindow(), nullptr, &m_surface));

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

	// swapchain & allocator
	m_swapchain.init();
	m_swapchain.create(window->getExtent(), VK_FORMAT_B8G8R8A8_UNORM);

	m_allocator.init();
	m_allocator.createDrawImage(window->getExtent(), &m_drawImage);

	m_deletionQueue.push_function([this] {
		m_allocator.destroyImage(m_drawImage);
		m_swapchain.destroy();
		m_allocator.destroy();
	});

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

	m_deletionQueue.push_function([this, &shader] { vkDestroyShaderModule(m_device, shader.m_shaderModule, nullptr); });

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

void VkeDevice::destroy() {
	m_deletionQueue.flush();

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

	vkDestroyDevice(m_device, nullptr);
	vkb::destroy_debug_utils_messenger(m_instance, m_debugMessenger);
	vkDestroyInstance(m_instance, nullptr);
}

} // namespace vke
