#pragma once

#include "vke_pipelines.hpp"
#include "vke_utils.hpp"
#include "vke_window.hpp"
#include "vke_swapchain.hpp"
#include "vke_types.hpp"

namespace vke {

struct FrameData;

class VkeDevice {

	friend class VkeSwapchain;
	friend class VkeAllocator;

public:
	VkeDevice(){};

	VkResult init(VkeWindow* window);
	void destroy();

	void waitIdle() { vkDeviceWaitIdle(m_device); }

	VkDevice getDevice() { return m_device; }

public:
	VkResult createCommandPool(VkCommandPool* pool, VkCommandPoolCreateFlags flags = 0);
	VkResult allocateCommandBuffer(VkCommandBuffer* buffer, VkCommandPool pool);
	VkResult createSemaphore(VkSemaphore* semaphore, VkSemaphoreCreateFlags flags = 0);
	VkResult createFence(VkFence* fence, VkFenceCreateFlags flags = 0);
	VkResult initFrameData(FrameData* frame, int count);
	VkResult createShader(VkeShader& shader, const char* path);
	VkResult createGraphicsPipeline(VkeGraphicsPipeline& pipeline);
	VkResult createDrawImage(VkExtent2D extent, AllocatedImage* image);
	VkResult submitCommand(int submitCount, VkSubmitInfo2* submitInfo, VkFence fence = VK_NULL_HANDLE);

private:
	VkInstance m_vkInstance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	VkSurfaceKHR m_surface;
	VkPhysicalDevice m_chosenGPU;
	VkDevice m_device;

	VmaAllocator m_allocator;

	VkQueue m_graphicsQueue;
	uint32_t m_graphicsQueueFamily;

	vkutil::DeletionQueue m_deletionQueue;
};

} // namespace vke
