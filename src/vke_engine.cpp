#include "vke_images.hpp"
#include <VkBootstrap.h>
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vke_engine.hpp"
#include "vke_initializers.hpp"

namespace vke {

constexpr bool useValidationLayers = true;

void VkEngine::init() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = glfwCreateWindow(m_windowExtent.width, m_windowExtent.height, "Vulkan Engine", nullptr, nullptr);

	initVulkan();

	initSwapchain();

	initCommands();

	initSyncStructures();

	m_initiliazed = true;
}

void VkEngine::initVulkan() {
	// instance
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
	VK_CHECK(glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface));

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
}

void VkEngine::initSwapchain() {
	m_swapchain.init();
	m_swapchain.create(m_windowExtent, VK_FORMAT_B8G8R8A8_UNORM);

	m_allocator.init();
	m_allocator.createDrawImage(m_windowExtent, &m_drawImage);

	m_deletionQueue.push_function([this] {
		m_allocator.destroyImage(m_drawImage);
		m_swapchain.destroy();
		m_allocator.destroy();
	});
}

void VkEngine::initCommands() {
	auto commandPoolCreateInfo =
		vkinit::commandPoolCreateInfo(m_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	for (uint32_t i = 0; i < FRAME_OVERLAP; i++) {
		VK_CHECK(vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &m_frames[i]._commandPool));

		auto cmdAllocInfo = vkinit::commandBufferAllocateInfo(m_frames[i]._commandPool);
		VK_CHECK(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &m_frames[i]._commandBuffer));

		m_deletionQueue.push_function([this, i] {
			vkDestroyCommandPool(m_device, m_frames[i]._commandPool, nullptr);
			m_frames[i]._deletionQueue.flush();
		});
	}
}

void VkEngine::initSyncStructures() {
	auto fenceCreateInfo = vkinit::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	auto semaphoreCreateInfo = vkinit::semaphoreCreateInfo();

	for (uint32_t i = 0; i < FRAME_OVERLAP; i++) {
		VK_CHECK(vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_frames[i]._renderFence));

		VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_frames[i]._swapchainSemaphore));
		VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_frames[i]._renderSemaphore));

		m_deletionQueue.push_function([this, i] {
			vkDestroyFence(m_device, m_frames[i]._renderFence, nullptr);
			vkDestroySemaphore(m_device, m_frames[i]._swapchainSemaphore, nullptr);
			vkDestroySemaphore(m_device, m_frames[i]._renderSemaphore, nullptr);
		});
	}
}

void VkEngine::run() {
	bool quit = false;

	while (!quit) {
		glfwPollEvents();

		if (glfwWindowShouldClose(m_window) || glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			quit = true;

		draw();
	}
}

void VkEngine::draw() {
	// sync
	VK_CHECK(vkWaitForFences(m_device, 1, &getCurrentFrame()._renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(m_device, 1, &getCurrentFrame()._renderFence));

	getCurrentFrame()._deletionQueue.flush();

	// get screen image
	m_swapchain.acquireImage(getCurrentFrame()._swapchainSemaphore);

	// prepare to recording
	VkCommandBuffer cmd = getCurrentFrame()._commandBuffer;

	VK_CHECK(vkResetCommandBuffer(cmd, 0));

	m_drawExtent.width = m_drawImage.imageExtent.width;
	m_drawExtent.height = m_drawImage.imageExtent.height;

	// record commands
	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	vkutil::makeWriteable(cmd, m_drawImage);

	/* draw something here for test */

	// copy onto swapchain
	vkutil::copyImageToImage(cmd, m_drawImage, m_swapchain.getCurrentImage(), m_drawExtent, m_swapchain.getExtent());
	vkutil::makePresentable(cmd, m_swapchain.getCurrentImage());

	VK_CHECK(vkEndCommandBuffer(cmd));

	// submit to graphics queue
	VkSemaphoreSubmitInfo waitInfo =
		vkinit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, getCurrentFrame()._swapchainSemaphore);
	VkSemaphoreSubmitInfo signalInfo =
		vkinit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, getCurrentFrame()._renderSemaphore);

	VkCommandBufferSubmitInfo cmdinfo = vkinit::commandBufferSubmitInfo(cmd);
	VkSubmitInfo2 submitInfo = vkinit::submitInfo(&cmdinfo, &signalInfo, &waitInfo);

	VK_CHECK(vkQueueSubmit2(m_graphicsQueue, 1, &submitInfo, getCurrentFrame()._renderFence));

	// submit to presentation queue
	m_swapchain.presentOnScreen(getCurrentFrame()._renderSemaphore);

	m_frame++;
}

void VkEngine::destroy() {
	if (!m_initiliazed)
		return;

	vkDeviceWaitIdle(m_device);

	m_deletionQueue.flush();

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

	vkDestroyDevice(m_device, nullptr);
	vkb::destroy_debug_utils_messenger(m_instance, m_debugMessenger);
	vkDestroyInstance(m_instance, nullptr);

	glfwDestroyWindow(m_window);
}

} // namespace vke
