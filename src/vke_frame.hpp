#pragma once

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

} // namespace vke
