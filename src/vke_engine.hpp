#pragma once

#include "vke_allocator.hpp"
#include "vke_images.hpp"
#include "vke_swapchain.hpp"
#include "vke_types.hpp"
#include "vke_utils.hpp"

namespace vke {

struct FrameData {
	VkSemaphore _swapchainSemaphore, _renderSemaphore;
	VkFence _renderFence;

	VkCommandPool _commandPool;
	VkCommandBuffer _commandBuffer;

	vkutil::DeletionQueue _deletionQueue;
};

class VkEngine {
public:
	bool m_initiliazed{false};
	int m_frame{0};

	static constexpr unsigned int FRAME_OVERLAP = 2;

	VkExtent2D m_windowExtent{700, 800};
	GLFWwindow* m_window;

	void init();
	void run();
	void destroy();

	void draw();

	friend class VkeSwapchain;
	friend class VkeAllocator;

	VkeSwapchain m_swapchain{this};
	VkeAllocator m_allocator{this};

private:
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	VkSurfaceKHR m_surface;
	VkPhysicalDevice m_chosenGPU;
	VkDevice m_device;

	VkQueue m_graphicsQueue;
	uint32_t m_graphicsQueueFamily;

	FrameData m_frames[FRAME_OVERLAP];
	FrameData& getCurrentFrame() { return m_frames[m_frame % FRAME_OVERLAP]; }

	AllocatedImage m_drawImage;
	VkExtent2D m_drawExtent;

	vkutil::DeletionQueue m_deletionQueue;

private:
	void initVulkan();
	void initSwapchain();
	void initCommands();
	void initSyncStructures();
};

} // namespace vke
