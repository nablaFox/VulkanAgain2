#pragma once

#include "vke_types.hpp"
#include <vulkan/vulkan.h>

namespace vkutil {

void copyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize);
void copyImageToImage(VkCommandBuffer cmd, VkeImage& src, VkeImage& dst, VkExtent2D srcSize, VkExtent2D dstSize);
void transitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);

void makeColorWriteable(VkCommandBuffer buffer, VkeImage& image);
void makeWriteable(VkCommandBuffer buffer, VkeImage& image);
void makePresentable(VkCommandBuffer buffer, VkeImage& image);
void makeTransferable(VkCommandBuffer buffer, VkeImage& image);
void makeCopyable(VkCommandBuffer buffer, VkeImage& image);

void setViewport(VkCommandBuffer cmd, VkExtent2D extent, float minDepth = 0.0f, float maxDepth = 1.0f, int x = 0, int y = 0);
void setScissor(VkCommandBuffer cmd, VkExtent2D extent, int x = 0, int y = 0);

} // namespace vkutil
