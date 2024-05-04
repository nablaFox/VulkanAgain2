#pragma once

#include <fmt/core.h>

#include <vulkan/vk_enum_string_helper.h>

#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#include <GLFW/glfw3.h>

#include <vk_mem_alloc.h>

struct VkeImage {
	VkImageLayout currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImage image;
	VkImageView imageView;
};

struct AllocatedImage : VkeImage {
	VmaAllocation allocation;
	VkExtent2D imageExtent;
	VkFormat imageFormat;
};

#define VK_CHECK(x)                                                                                                              \
	do {                                                                                                                         \
		VkResult err = x;                                                                                                        \
		if (err) {                                                                                                               \
			fmt::print("Detected Vulkan Error: {}", string_VkResult(err));                                                       \
			abort();                                                                                                             \
		}                                                                                                                        \
	} while (0)
