#include "vke_initializers.hpp"
#include "vke_images.hpp"
#include <vulkan/vulkan_core.h>

namespace vkutil {

void transitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout) {
	VkImageMemoryBarrier2 imageBarrier{.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
	imageBarrier.pNext = nullptr;

	imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
	imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

	imageBarrier.oldLayout = currentLayout;
	imageBarrier.newLayout = newLayout;

	VkImageAspectFlags aspectMask =
		(newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	imageBarrier.subresourceRange = vkinit::imageSubResourceRange(aspectMask);
	imageBarrier.image = image;

	VkDependencyInfo depInfo{};
	depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	depInfo.pNext = nullptr;

	depInfo.imageMemoryBarrierCount = 1;
	depInfo.pImageMemoryBarriers = &imageBarrier;

	vkCmdPipelineBarrier2(cmd, &depInfo);
}

void copyImageToImage(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize) {
	VkImageBlit2 blitRegion{.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr};

	blitRegion.srcOffsets[1].x = srcSize.width;
	blitRegion.srcOffsets[1].y = srcSize.height;
	blitRegion.srcOffsets[1].z = 1;

	blitRegion.dstOffsets[1].x = dstSize.width;
	blitRegion.dstOffsets[1].y = dstSize.height;
	blitRegion.dstOffsets[1].z = 1;

	blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.srcSubresource.baseArrayLayer = 0;
	blitRegion.srcSubresource.layerCount = 1;
	blitRegion.srcSubresource.mipLevel = 0;

	blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.dstSubresource.baseArrayLayer = 0;
	blitRegion.dstSubresource.layerCount = 1;
	blitRegion.dstSubresource.mipLevel = 0;

	VkBlitImageInfo2 blitInfo{.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr};
	blitInfo.dstImage = destination;
	blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	blitInfo.srcImage = source;
	blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	blitInfo.filter = VK_FILTER_LINEAR;
	blitInfo.regionCount = 1;
	blitInfo.pRegions = &blitRegion;

	vkCmdBlitImage2(cmd, &blitInfo);
}

void makeWriteable(VkCommandBuffer cmd, VkeImage& image) {
	transitionImage(cmd, image.image, image.currentLayout, VK_IMAGE_LAYOUT_GENERAL);
	image.currentLayout = VK_IMAGE_LAYOUT_GENERAL;
}

void makeColorWriteable(VkCommandBuffer buffer, VkeImage& image) {
	transitionImage(buffer, image.image, image.currentLayout, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	image.currentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}

void makePresentable(VkCommandBuffer cmd, VkeImage& image) {
	transitionImage(cmd, image.image, image.currentLayout, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	image.currentLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
}

void makeTransferable(VkCommandBuffer cmd, VkeImage& image) {
	transitionImage(cmd, image.image, image.currentLayout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	image.currentLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
}

void makeCopyable(VkCommandBuffer cmd, VkeImage& image) {
	transitionImage(cmd, image.image, image.currentLayout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	image.currentLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
}

void copyImageToImage(VkCommandBuffer cmd, VkeImage& src, VkeImage& dst, VkExtent2D srcSize, VkExtent2D dstSize) {
	makeTransferable(cmd, src);
	makeCopyable(cmd, dst);

	copyImageToImage(cmd, src.image, dst.image, srcSize, dstSize);
}

void setViewport(VkCommandBuffer cmd, VkExtent2D extent, float minDepth, float maxDepth, int x, int y) {
	VkViewport viewport = {};
	viewport.x = x;
	viewport.y = y;
	viewport.width = extent.width;
	viewport.height = extent.height;
	viewport.minDepth = minDepth;
	viewport.maxDepth = maxDepth;

	vkCmdSetViewport(cmd, 0, 1, &viewport);
}

void setScissor(VkCommandBuffer cmd, VkExtent2D extent, int x, int y) {
	VkRect2D scissor = {};
	scissor.offset.x = x;
	scissor.offset.y = y;
	scissor.extent.width = extent.width;
	scissor.extent.height = extent.height;

	vkCmdSetScissor(cmd, 0, 1, &scissor);
}

} // namespace vkutil
