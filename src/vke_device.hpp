#pragma once

#include "vke_pipelines.hpp"
#include "vke_frame.hpp"
#include "vke_utils.hpp"
#include "vke_window.hpp"
#include "vke_swapchain.hpp"
#include "vke_types.hpp"
#include "vke_allocator.hpp"

namespace vke {

class VkeDevice {

	friend class VkeSwapchain;
	friend class VkeAllocator;

protected:
	VkeDevice() {}

public:
	static constexpr int FRAME_OVERLAP = 2;

	void static create(VkeDevice* device);
	VkResult initialize(VkeWindow* window); // TODO: return a VkResult
	void destroy();

public:
	VkResult createCommandPool(VkCommandPool* pool, VkCommandPoolCreateFlags flags = 0);
	VkResult allocateCommandBuffer(VkCommandBuffer* buffer, VkCommandPool pool);
	VkResult createSemaphore(VkSemaphore* semaphore, VkSemaphoreCreateFlags flags = 0);
	VkResult createFence(VkFence* fence, VkFenceCreateFlags flags = 0);
	VkResult initFrameData(FrameData* frame, int count);
	VkResult createShader(VkeShader& shader, const char* path);
	VkResult createGraphicsPipeline(VkeGraphicsPipeline& pipeline);

	void waitIdle() { vkDeviceWaitIdle(m_device); }
	VkDevice getDevice() { return m_device; }
	const AllocatedImage& getDrawContext() { return m_drawImage; }

private:
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	VkSurfaceKHR m_surface;
	VkPhysicalDevice m_chosenGPU;
	VkDevice m_device;

	VkeSwapchain m_swapchain{this};
	VkeAllocator m_allocator{this};

	AllocatedImage m_drawImage;
	VkExtent2D m_drawExtent;

	VkQueue m_graphicsQueue;
	uint32_t m_graphicsQueueFamily;

	vkutil::DeletionQueue m_deletionQueue;
};

} // namespace vke
