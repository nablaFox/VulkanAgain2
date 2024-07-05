#pragma once

#include "vke_descriptors.hpp"
#include "vke_pipelines.hpp"
#include "vke_utils.hpp"
#include "vke_window.hpp"
#include "vke_swapchain.hpp"
#include "vke_types.hpp"

namespace vke {

struct ImmediateData {
	VkCommandBuffer _commandBuffer;
	VkCommandPool _commandPool;
	VkFence _fence;
};

class VkeDevice {

	friend class VkeSwapchain;

public:
	VkeDevice(){};

	VkResult init(VkeWindow* window);
	void destroy();

	void waitIdle() { vkDeviceWaitIdle(m_device); }

	VkDevice getDevice() { return m_device; }

public:
public:
	VkResult createCommandPool(VkCommandPool* pool, VkCommandPoolCreateFlags flags = 0);
	VkResult allocateCommandBuffer(VkCommandBuffer* buffer, VkCommandPool pool);
	VkResult createSemaphore(VkSemaphore* semaphore, VkSemaphoreCreateFlags flags = 0);
	VkResult createFence(VkFence* fence, VkFenceCreateFlags flags = 0);
	VkResult createShader(VkeShader& shader, const char* path);
	VkResult destroyShader(VkeShader& shader);
	VkResult createPipelineLayout(VkePipeline& pipeline, VkPipelineLayoutCreateInfo& layoutInfo);
	VkResult createGraphicsPipeline(VkeGraphicsPipeline& pipeline);
	VkResult createComputePipeline(VkeComputePipeline& pipeline);
	VkResult createDrawImage(VkExtent2D extent, AllocatedImage* image);
	VkResult submitCommand(int submitCount, VkSubmitInfo2* submitInfo, VkFence fence = VK_NULL_HANDLE);
	VkResult immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

	VkResult uploadMesh(GPUMeshBuffers* mesh, std::span<uint32_t> indices, std::span<Vertex> vertices);

	VkResult createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, AllocatedBuffer* buffer,
						  bool temp = false);
	VkResult fillBuffer(AllocatedBuffer* buffer, void* data, size_t size);
	VkResult createStagingBuffer(size_t allocSize, AllocatedBuffer* buffer, void*& data);
	VkResult destroyBuffer(AllocatedBuffer* buffer);

	VkResult createImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, AllocatedImage* handle,
						 bool mipmapped = false);
	VkResult fillImage(AllocatedImage* image, void* data);
	VkResult createFilledImage(AllocatedImage* image, void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage);
	VkResult destroyImage(AllocatedImage* image);

	VkResult createSampler(VkSampler* sampler, VkSamplerCreateInfo* info);
	VkResult destroySampler(VkSampler* sampler);

	VkResult initDescriptorSetLayout(VkeDescriptor* descriptorSet, VkShaderStageFlags shaderStages);
	VkResult allocateDescriptorSet(VkeDescriptor* descriptorSet, VkeDescriptorAllocator* allocator, bool temp = false);
	VkResult initDescriptorPool(VkeDescriptorAllocator* descriptorAllocator, uint32_t maxSets,
								std::span<VkeDescriptorAllocator::PoolSizeRatio> poolRatios);
	VkResult destroyDescriptorPool(VkeDescriptorAllocator* descriptorAllocator);
	VkResult resetDescriptorPool(VkeDescriptorAllocator* descriptorAllocator);

private:
	VkInstance m_vkInstance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	VkSurfaceKHR m_surface;
	VkPhysicalDevice m_chosenGPU;
	VkDevice m_device;
	ImmediateData m_immData;

	VmaAllocator m_allocator;

	VkQueue m_graphicsQueue;
	uint32_t m_graphicsQueueFamily;

	vkutil::DeletionQueue m_deletionQueue;
};

} // namespace vke
