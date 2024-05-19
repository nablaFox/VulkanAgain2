#pragma once

#include <fmt/core.h>

#include <vulkan/vk_enum_string_helper.h>

#include <span>
#include <deque>
#include <functional>
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

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

struct AllocatedBuffer {
	VkBuffer buffer;
	VmaAllocation allocation;
	VmaAllocationInfo info;
};

struct GPUMeshBuffers {
	AllocatedBuffer indexBuffer;
	AllocatedBuffer vertexBuffer;
	VkDeviceAddress vertexBufferAddress;
};

struct GPUDrawPushConstants {
	glm::mat4 worldMatrix;
	VkDeviceAddress vertexBuffer;
};

struct Vertex {
	glm::vec3 position;
	float uv_x;
	glm::vec3 normal;
	float uv_y;
	glm::vec4 color;
};

#define VK_RETURN(x)                                                                                                             \
	{                                                                                                                            \
		VkResult err = x;                                                                                                        \
		if (err != VK_SUCCESS)                                                                                                   \
			return err;                                                                                                          \
	}

#define VK_CHECK(x)                                                                                                              \
	do {                                                                                                                         \
		VkResult err = x;                                                                                                        \
		if (err) {                                                                                                               \
			fmt::print("Detected Vulkan Error: {}", string_VkResult(err));                                                       \
			abort();                                                                                                             \
		}                                                                                                                        \
	} while (0)
