#pragma once

#include "vke_types.hpp"
#include <vulkan/vulkan_core.h>

namespace vke {

class VkeDevice;

class VkeSwapchain {

public:
	VkeSwapchain(VkeDevice* device) : m_device(device) {}
	~VkeSwapchain();

	void init();

	void create(VkExtent2D extent, VkFormat format);

	void destroy();

	void acquireImage(VkSemaphore semaphore = nullptr);
	VkeImage& getCurrentImage() { return m_swapchainImages[m_currentImage]; }
	VkExtent2D getExtent() { return m_swapchainExtent; }
	void presentOnScreen(VkSemaphore semaphore = nullptr);

	VkSwapchainKHR& get_swapchain() { return m_swapchain; }

private:
	VkeDevice* m_device;

	VkSwapchainKHR m_swapchain;
	VkFormat m_swapchainImageFormat;
	VkExtent2D m_swapchainExtent;

	std::vector<VkeImage> m_swapchainImages;

	VkPresentInfoKHR m_presentInfo{};
	uint32_t m_currentImage;
};

} // namespace vke
