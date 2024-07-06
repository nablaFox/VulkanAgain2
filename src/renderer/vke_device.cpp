#include "vke_device.hpp"
#include "vke_images.hpp"
#include "vke_initializers.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

using namespace vke;

constexpr bool useValidationLayers = true; // TODO: handle debug mode in a better way

VkResult VkeDevice::init(VkeWindow* window) {
	vkb::InstanceBuilder builder;

	auto instRet = builder.set_app_name("Vulkan Engine")
					   .request_validation_layers(useValidationLayers)
					   .use_default_debug_messenger()
					   .require_api_version(1, 3, 0)
					   .build();

	vkb::Instance vkbInst = instRet.value();

	m_vkInstance = vkbInst.instance;
	m_debugMessenger = vkbInst.debug_messenger;

	// physical and logical devices
	VK_RETURN(glfwCreateWindowSurface(m_vkInstance, window->getWindow(), nullptr, &m_surface));

	VkPhysicalDeviceVulkan13Features features{};
	features.dynamicRendering = true;
	features.synchronization2 = true;

	VkPhysicalDeviceVulkan12Features features12{};
	features12.bufferDeviceAddress = true;
	features12.descriptorIndexing = true;

	vkb::PhysicalDeviceSelector selector{vkbInst};
	vkb::PhysicalDevice phyisicalDevice = selector.set_minimum_version(1, 3)
											  .set_required_features_13(features)
											  .set_required_features_12(features12)
											  .set_surface(m_surface)
											  .select()
											  .value();

	vkb::DeviceBuilder deviceBuilder{phyisicalDevice};
	vkb::Device vkbDevice = deviceBuilder.build().value();

	m_chosenGPU = vkbDevice.physical_device;
	m_device = vkbDevice.device;

	m_graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	m_graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

	VmaAllocatorCreateInfo allocatorInfo = {
		.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
		.physicalDevice = m_chosenGPU,
		.device = m_device,
		.instance = m_vkInstance,
	};

	VK_RETURN(vmaCreateAllocator(&allocatorInfo, &m_allocator));

	VK_RETURN(createCommandPool(&m_immData._commandPool, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
	VK_RETURN(allocateCommandBuffer(&m_immData._commandBuffer, m_immData._commandPool));
	VK_RETURN(createFence(&m_immData._fence, VK_FENCE_CREATE_SIGNALED_BIT));

	m_deletionQueue.push_function([this] { vmaDestroyAllocator(m_allocator); });

	return VK_SUCCESS;
}

VkResult VkeDevice::createCommandPool(VkCommandPool* pool, VkCommandPoolCreateFlags flags) {
	auto commandPoolCreateInfo = vkinit::commandPoolCreateInfo(m_graphicsQueueFamily, flags);

	m_deletionQueue.push_function([this, pool] { vkDestroyCommandPool(m_device, *pool, nullptr); });

	return vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, pool);
}

VkResult VkeDevice::allocateCommandBuffer(VkCommandBuffer* buffer, VkCommandPool pool) {
	auto cmdAllocInfo = vkinit::commandBufferAllocateInfo(pool);
	return vkAllocateCommandBuffers(m_device, &cmdAllocInfo, buffer);
}

VkResult VkeDevice::createSemaphore(VkSemaphore* semaphore, VkSemaphoreCreateFlags flags) {
	auto semaphoreCreateInfo = vkinit::semaphoreCreateInfo(flags);

	m_deletionQueue.push_function([this, semaphore] { vkDestroySemaphore(m_device, *semaphore, nullptr); });

	return vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, semaphore);
}

VkResult VkeDevice::createFence(VkFence* fence, VkFenceCreateFlags flags) {
	auto fenceCreateInfo = vkinit::fenceCreateInfo(flags);

	m_deletionQueue.push_function([this, fence] { vkDestroyFence(m_device, *fence, nullptr); });

	return vkCreateFence(m_device, &fenceCreateInfo, nullptr, fence);
}

VkResult VkeDevice::createShader(VkeShader& shader, const char* path) {
	std::vector<uint32_t> buffer;
	VK_RETURN(shader.loadShaderModule(path, buffer));

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.codeSize = buffer.size() * sizeof(uint32_t);
	createInfo.pCode = buffer.data();

	VK_RETURN(vkCreateShaderModule(m_device, &createInfo, nullptr, &shader.m_shaderModule));

	fmt::println("Shader module created: {}", path);

	return VK_SUCCESS;
}

VkResult VkeDevice::destroyShader(VkeShader& shader) {
	vkDestroyShaderModule(m_device, shader.m_shaderModule, nullptr);
	return VK_SUCCESS;
}

VkResult VkeDevice::createPipelineLayout(VkePipeline& pipeline, VkPipelineLayoutCreateInfo& layoutInfo) {
	VK_RETURN(vkCreatePipelineLayout(m_device, &layoutInfo, nullptr, &pipeline.m_pipelineLayout));

	m_deletionQueue.push_function([this, &pipeline] { vkDestroyPipelineLayout(m_device, pipeline.m_pipelineLayout, nullptr); });

	return VK_SUCCESS;
}

VkResult VkeDevice::createGraphicsPipeline(VkeGraphicsPipeline& pipeline) {
	VK_RETURN(createPipelineLayout(pipeline, pipeline.m_pipelineLayoutInfo));

	VkPipelineViewportStateCreateInfo viewportState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.viewportCount = 1,
		.scissorCount = 1,
	};

	VkPipelineColorBlendStateCreateInfo colorBlending = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext = nullptr,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &pipeline.m_colorBlendAttachment,
	};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

	VkDynamicState state[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dynamicState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = state,
	};

	VkGraphicsPipelineCreateInfo pipelineInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &pipeline.m_renderInfo,
		.stageCount = (uint32_t)pipeline.m_shaderStages.size(),
		.pStages = pipeline.m_shaderStages.data(),
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &pipeline.m_inputAssembly,
		.pViewportState = &viewportState,
		.pRasterizationState = &pipeline.m_rasterizer,
		.pMultisampleState = &pipeline.m_multisampling,
		.pDepthStencilState = &pipeline.m_depthStencil,
		.pColorBlendState = &colorBlending,
		.pDynamicState = &dynamicState,
		.layout = pipeline.m_pipelineLayout,
	};

	VK_RETURN(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.m_pipeline));

	m_deletionQueue.push_function([this, &pipeline] { vkDestroyPipeline(m_device, pipeline.m_pipeline, nullptr); });

	return VK_SUCCESS;
}

VkResult VkeDevice::createComputePipeline(VkeComputePipeline& pipeline) {
	VK_RETURN(createPipelineLayout(pipeline, pipeline.m_pipelineLayoutInfo));

	pipeline.m_computeInfo.layout = pipeline.m_pipelineLayout;
	VK_RETURN(vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &pipeline.m_computeInfo, nullptr, &pipeline.m_pipeline));

	m_deletionQueue.push_function([this, &pipeline] { vkDestroyPipeline(m_device, pipeline.m_pipeline, nullptr); });

	return VK_SUCCESS;
}

VkResult VkeDevice::createDrawImage(VkExtent2D extent, AllocatedImage* image) {
	VkExtent3D drawImageExtent = {
		extent.width,
		extent.height,
		1,
	};

	image->imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	image->imageExtent = drawImageExtent;

	VkImageUsageFlags drawImageUsages{};
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	auto imageCreateInfo = vkinit::imageCreateInfo(image->imageFormat, drawImageUsages, drawImageExtent);

	VmaAllocationCreateInfo imageAllocInfo{};
	imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	imageAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vmaCreateImage(m_allocator, &imageCreateInfo, &imageAllocInfo, &image->image, &image->allocation, nullptr);

	auto imageViewCreateInfo = vkinit::imageViewCreateInfo(image->imageFormat, image->image, VK_IMAGE_ASPECT_COLOR_BIT);

	VK_RETURN(vkCreateImageView(m_device, &imageViewCreateInfo, nullptr, &image->imageView));

	m_deletionQueue.push_function([this, image] {
		vkDestroyImageView(m_device, image->imageView, nullptr);
		vmaDestroyImage(m_allocator, image->image, image->allocation);
	});

	return VK_SUCCESS;
}

VkResult VkeDevice::submitCommand(int submitCount, VkSubmitInfo2* submitInfo, VkFence fence) {
	VK_RETURN(vkQueueSubmit2(m_graphicsQueue, submitCount, submitInfo, fence));
	return VK_SUCCESS;
}

VkResult VkeDevice::immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function) {
	VkCommandBuffer cmd = m_immData._commandBuffer;

	VK_CHECK(vkResetFences(m_device, 1, &m_immData._fence));
	VK_CHECK(vkResetCommandBuffer(cmd, 0));

	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	function(cmd);

	VK_CHECK(vkEndCommandBuffer(cmd));

	VkCommandBufferSubmitInfo cmdinfo = vkinit::commandBufferSubmitInfo(cmd);
	VkSubmitInfo2 submit = vkinit::submitInfo(&cmdinfo, nullptr, nullptr);

	VK_CHECK(submitCommand(1, &submit, m_immData._fence));
	VK_CHECK(vkWaitForFences(m_device, 1, &m_immData._fence, true, 9999999999));

	return VK_SUCCESS;
}

VkResult VkeDevice::uploadMesh(GPUMeshBuffers* mesh, std::span<uint32_t> indices, std::span<Vertex> vertices) {
	const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
	const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

	GPUMeshBuffers newSurface;
	VK_RETURN(createBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
						   VMA_MEMORY_USAGE_GPU_ONLY, &newSurface.indexBuffer));

	VK_RETURN(createBuffer(vertexBufferSize,
						   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
							   VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
						   VMA_MEMORY_USAGE_GPU_ONLY, &newSurface.vertexBuffer));

	VkBufferDeviceAddressInfo deviceAdressInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = newSurface.vertexBuffer.buffer,
	};

	newSurface.vertexBufferAddress = vkGetBufferDeviceAddress(m_device, &deviceAdressInfo);

	AllocatedBuffer staging;
	void* data;

	VK_RETURN(createStagingBuffer(vertexBufferSize + indexBufferSize, &staging, data));

	memcpy(data, vertices.data(), vertexBufferSize);
	memcpy((char*)data + vertexBufferSize, indices.data(), indexBufferSize);

	immediateSubmit([&](VkCommandBuffer cmd) {
		VkBufferCopy vertexCopy{
			.srcOffset = 0,
			.dstOffset = 0,
			.size = vertexBufferSize,
		};

		VkBufferCopy indexCopy{
			.srcOffset = vertexBufferSize,
			.dstOffset = 0,
			.size = indexBufferSize,
		};

		vkCmdCopyBuffer(cmd, staging.buffer, newSurface.vertexBuffer.buffer, 1, &vertexCopy);
		vkCmdCopyBuffer(cmd, staging.buffer, newSurface.indexBuffer.buffer, 1, &indexCopy);
	});

	VK_RETURN(destroyBuffer(&staging));

	*mesh = newSurface;

	return VK_SUCCESS;
}

VkResult VkeDevice::createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, AllocatedBuffer* buffer,
								 bool temp) {
	VkBufferCreateInfo bufferInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = nullptr,
		.size = allocSize,
		.usage = usage,
	};

	VmaAllocationCreateInfo vmaallocInfo = {
		.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
		.usage = memoryUsage,
	};

	VK_RETURN(vmaCreateBuffer(m_allocator, &bufferInfo, &vmaallocInfo, &buffer->buffer, &buffer->allocation, &buffer->info));

	if (!temp) {
		auto bufferPtr = new AllocatedBuffer(*buffer);
		m_deletionQueue.push_function([this, bufferPtr]() {
			destroyBuffer(bufferPtr);
			delete bufferPtr;
		});
	}

	return VK_SUCCESS;
}

VkResult VkeDevice::fillBuffer(AllocatedBuffer* buffer, void* data, size_t size) {
	memcpy(buffer->allocation->GetMappedData(), data, size);
	return VK_SUCCESS;
}

VkResult VkeDevice::createStagingBuffer(size_t allocSize, AllocatedBuffer* staging, void*& data) {
	VK_RETURN(createBuffer(allocSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, staging, true));
	data = staging->allocation->GetMappedData();
	return VK_SUCCESS;
}

VkResult VkeDevice::destroyBuffer(AllocatedBuffer* buffer) {
	vmaDestroyBuffer(m_allocator, buffer->buffer, buffer->allocation);
	return VK_SUCCESS;
}

VkResult VkeDevice::createImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, AllocatedImage* handle,
								bool mipmapped) {
	handle->imageFormat = format;
	handle->imageExtent = size;

	VkImageCreateInfo imgInfo = vkinit::imageCreateInfo(format, usage, size);
	if (mipmapped) {
		imgInfo.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
	}

	VmaAllocationCreateInfo allocInfo = {
		.usage = VMA_MEMORY_USAGE_GPU_ONLY,
		.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
	};

	VK_RETURN(vmaCreateImage(m_allocator, &imgInfo, &allocInfo, &handle->image, &handle->allocation, nullptr));

	VkImageAspectFlags aspectFlag = format == VK_FORMAT_D32_SFLOAT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	VkImageViewCreateInfo viewInfo = vkinit::imageViewCreateInfo(format, handle->image, aspectFlag);
	viewInfo.subresourceRange.levelCount = imgInfo.mipLevels;

	VK_RETURN(vkCreateImageView(m_device, &viewInfo, nullptr, &handle->imageView));

	m_deletionQueue.push_function([this, handle] {
		vkDestroyImageView(m_device, handle->imageView, nullptr);
		vmaDestroyImage(m_allocator, handle->image, handle->allocation);
	});

	return VK_SUCCESS;
}

VkResult VkeDevice::fillImage(AllocatedImage* image, void* data) {
	size_t imageSize = image->imageExtent.width * image->imageExtent.height * image->imageExtent.depth * 4;

	AllocatedBuffer stagingBuffer;
	VK_RETURN(createStagingBuffer(imageSize, &stagingBuffer, data));

	immediateSubmit([&](VkCommandBuffer cmd) {
		vkutil::transitionImage(cmd, image->image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;

		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageExtent = image->imageExtent;

		vkCmdCopyBufferToImage(cmd, stagingBuffer.buffer, image->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		vkutil::transitionImage(cmd, image->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
								VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	});

	VK_RETURN(destroyBuffer(&stagingBuffer));

	return VK_SUCCESS;
}

VkResult VkeDevice::createFilledImage(AllocatedImage* image, void* data, VkExtent3D size, VkFormat format,
									  VkImageUsageFlags usage) {
	VK_RETURN(createImage(size, format, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, image, false));
	VK_RETURN(fillImage(image, data));
	return VK_SUCCESS;
}

VkResult VkeDevice::destroyImage(AllocatedImage* image) {
	vkDestroyImageView(m_device, image->imageView, nullptr);
	vmaDestroyImage(m_allocator, image->image, image->allocation);
	return VK_SUCCESS;
}

VkResult VkeDevice::createSampler(VkSampler* sampler, VkSamplerCreateInfo* samplerInfo) {
	VK_RETURN(vkCreateSampler(m_device, samplerInfo, nullptr, sampler));

	m_deletionQueue.push_function([this, sampler] { destroySampler(sampler); });

	return VK_SUCCESS;
}

VkResult VkeDevice::destroySampler(VkSampler* sampler) {
	vkDestroySampler(m_device, *sampler, nullptr);
	return VK_SUCCESS;
}

VkResult VkeDevice::initDescriptorSetLayout(VkeDescriptor* descriptorSet, VkShaderStageFlags shaderStages) {
	VK_RETURN(descriptorSet->initLayout(m_device, shaderStages));

	m_deletionQueue.push_function(
		[this, descriptorSet] { vkDestroyDescriptorSetLayout(m_device, descriptorSet->m_descriptorSetLayout, nullptr); });

	return VK_SUCCESS;
}

VkResult VkeDevice::allocateDescriptorSet(VkeDescriptor* descriptorSet, VkeDescriptorAllocator* allocator, bool temp) {
	VK_RETURN(allocator->allocate(m_device, descriptorSet));

	if (!temp) {
		m_deletionQueue.push_function([this, allocator, descriptorSet] {
			vkFreeDescriptorSets(m_device, allocator->m_pool, 1, &descriptorSet->m_descriptorSet);
		});
	}

	return VK_SUCCESS;
}

VkResult VkeDevice::initDescriptorPool(VkeDescriptorAllocator* allocator, uint32_t maxSets,
									   std::span<VkeDescriptorAllocator::PoolSizeRatio> poolRatios) {
	VK_RETURN(allocator->initPool(m_device, maxSets, poolRatios));

	m_deletionQueue.push_function([this, allocator] { destroyDescriptorPool(allocator); });

	return VK_SUCCESS;
}

VkResult VkeDevice::destroyDescriptorPool(VkeDescriptorAllocator* allocator) {
	allocator->destroyPoll(m_device);
	return VK_SUCCESS;
}

VkResult VkeDevice::resetDescriptorPool(VkeDescriptorAllocator* allocator) { return allocator->resetDescriptorPool(m_device); }

void VkeDevice::destroy() {
	m_deletionQueue.flush();

	vkDestroySurfaceKHR(m_vkInstance, m_surface, nullptr);

	vkDestroyDevice(m_device, nullptr);
	vkb::destroy_debug_utils_messenger(m_vkInstance, m_debugMessenger);
	vkDestroyInstance(m_vkInstance, nullptr);
}
