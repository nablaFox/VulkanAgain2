#include "vke_engine.hpp"
#include "vke_images.hpp"
#include "vke_swapchain.hpp"
#include <vulkan/vulkan_core.h>

namespace vke {

void VkeSwapchain::init(VkeDevice* device, VkExtent2D extent, VkFormat format) {
	m_device = device;
	m_swapchainImageFormat = format;
	m_presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	m_presentInfo.pNext = nullptr;
	m_presentInfo.pSwapchains = &m_swapchain;
	m_presentInfo.swapchainCount = 1;
	m_presentInfo.pImageIndices = &m_currentImage;

	VkSurfaceFormatKHR swapchainFormat{
		.format = m_swapchainImageFormat,
		.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
	};

	vkb::SwapchainBuilder swapchainBuilder{m_device->m_chosenGPU, m_device->m_device, m_device->m_surface};
	vkb::Swapchain vkbSwapchain = swapchainBuilder.set_desired_format(swapchainFormat)
									  .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
									  .set_desired_extent(extent.width, extent.height)
									  .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
									  .build()
									  .value();

	m_swapchainExtent = vkbSwapchain.extent;
	m_swapchain = vkbSwapchain.swapchain;

	auto images = vkbSwapchain.get_images().value();
	auto views = vkbSwapchain.get_image_views().value();

	for (uint32_t i = 0; i < images.size(); ++i) {
		VkeImage image{.image = images[i], .imageView = views[i]};
		m_swapchainImages.push_back(std::move(image));
	}
}

void VkeSwapchain::acquireImage(VkSemaphore semaphore) {
	VK_CHECK(vkAcquireNextImageKHR(m_device->m_device, m_swapchain, 1000000000, semaphore, nullptr, &m_currentImage));
}

void VkeSwapchain::presentOnScreen(VkSemaphore semaphore) {
	m_presentInfo.waitSemaphoreCount = 1;
	m_presentInfo.pWaitSemaphores = &semaphore;

	VK_CHECK(vkQueuePresentKHR(m_device->m_graphicsQueue, &m_presentInfo));
}

VkeSwapchain::~VkeSwapchain() { assert(m_device == nullptr); }

void VkeSwapchain::destroy() {
	vkDestroySwapchainKHR(m_device->m_device, m_swapchain, nullptr);

	for (auto image : m_swapchainImages)
		vkDestroyImageView(m_device->m_device, image.imageView, nullptr);

	m_device = nullptr;
}

} // namespace vke
