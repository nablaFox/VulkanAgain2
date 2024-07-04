#include "vke_pipelines.hpp"
#include "vke_engine.hpp"
#include "vke_images.hpp"
#include "vke_initializers.hpp"

using namespace vke;

void VkEngine::init(GameEngineSettings settings) {
	VK_CHECK(m_window.init(settings.appName, settings.windowWidth, settings.windowHeight));

	VK_CHECK(m_device.init(&m_window));

	m_swapchain.init(&m_device, m_window.getExtent(), VK_FORMAT_B8G8R8A8_UNORM);

	std::vector<VkeDescriptorAllocator::PoolSizeRatio> frameSizes = {
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3},
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4},
	};

	for (uint32_t i = 0; i < FRAME_OVERLAP; i++) {
		VK_CHECK(m_device.createCommandPool(&m_frames[i]._commandPool, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
		VK_CHECK(m_device.allocateCommandBuffer(&m_frames[i]._commandBuffer, m_frames[i]._commandPool));

		VK_CHECK(m_device.createSemaphore(&m_frames[i]._swapchainSemaphore));
		VK_CHECK(m_device.createSemaphore(&m_frames[i]._renderSemaphore));
		VK_CHECK(m_device.createFence(&m_frames[i]._renderFence, VK_FENCE_CREATE_SIGNALED_BIT));

		VK_CHECK(m_device.initDescriptorPool(&m_frames[i]._descriptorAllocator, 100, frameSizes));
	}

	std::vector<VkeDescriptorAllocator::PoolSizeRatio> globalSizes = {
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},
	};

	VK_CHECK(m_device.initDescriptorPool(&m_globalDescriptorAllocator, 10, globalSizes));

	VK_CHECK(m_device.createDrawImage(m_window.getExtent(), &m_drawImage));

	initPipelines();
	initTestData();

	fmt::println("Engine initialized");

	m_initiliazed = true;
}

void VkEngine::initPipelines() {
	VK_CHECK(m_device.createShader(m_vertexShader, "shaders/basic.vert.spv"));
	VK_CHECK(m_device.createShader(m_fragmentShader, "shaders/basic.frag.spv"));
	VK_CHECK(m_device.createShader(m_computeShader, "shaders/basic.comp.spv"));

	auto bufferRange = vkutil::getPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(GPUDrawPushConstants));

	m_globalSceneDescriptor.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	VK_CHECK(
		m_device.initDescriptorSetLayout(&m_globalSceneDescriptor, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT));

	m_materialDescriptor.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	m_materialDescriptor.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	m_materialDescriptor.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	VK_CHECK(m_device.initDescriptorSetLayout(&m_materialDescriptor, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT));

	m_meshPipeline.setShaders(m_vertexShader, m_fragmentShader)
		.setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
		.setPolygonMode(VK_POLYGON_MODE_FILL)
		.setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE)
		.setMultisamplingNone()
		.disableBlending()
		.disableDepthTest()
		.setColorAttachmentFormat(m_drawImage.imageFormat)
		.setPushConstantRange(bufferRange)
		.setDescriptorSet(m_globalSceneDescriptor);
	// .setDescriptorSet(m_materialDescriptor);

	m_drawImageDescriptor.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	VK_CHECK(m_device.initDescriptorSetLayout(&m_drawImageDescriptor, VK_SHADER_STAGE_COMPUTE_BIT));

	m_drawImageDescriptor.writeImage(0, m_drawImage.imageView, nullptr, VK_IMAGE_LAYOUT_GENERAL);
	VK_CHECK(m_device.allocateDescriptorSet(&m_drawImageDescriptor, &m_globalDescriptorAllocator));

	m_computePipeline.setShader(m_computeShader).setDescriptorSet(m_drawImageDescriptor);

	VK_CHECK(m_device.createGraphicsPipeline(m_meshPipeline));
	VK_CHECK(m_device.createComputePipeline(m_computePipeline));

	VK_CHECK(m_device.destroyShader(m_vertexShader));
	VK_CHECK(m_device.destroyShader(m_fragmentShader));
	VK_CHECK(m_device.destroyShader(m_computeShader));
}

// TEMP: this should be the entry point of the engine for the user code
void VkEngine::run() {
	bool quit = false;

	while (!quit) {
		glfwPollEvents();

		if (glfwWindowShouldClose(m_window.getWindow()))
			quit = true;

		startFrame();

		// drawGeometryTest();
		m_sceneManager.update(0.0f);

		endFrame();
	}
}

void VkEngine::startFrame() {
	VK_CHECK(vkWaitForFences(m_device.getDevice(), 1, &getCurrentFrame()._renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(m_device.getDevice(), 1, &getCurrentFrame()._renderFence));

	getCurrentFrame()._deletionQueue.flush();
	VK_CHECK(m_device.resetDescriptorPool(&getCurrentFrame()._descriptorAllocator));

	m_swapchain.acquireImage(getCurrentFrame()._swapchainSemaphore);

	VK_CHECK(vkResetCommandBuffer(currentCmd(), 0));

	m_drawExtent.width = m_drawImage.imageExtent.width;
	m_drawExtent.height = m_drawImage.imageExtent.height;

	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VK_CHECK(vkBeginCommandBuffer(currentCmd(), &cmdBeginInfo));
}

void VkEngine::endFrame() {
	vkutil::copyImageToImage(currentCmd(), m_drawImage, m_swapchain.getCurrentImage(), m_drawExtent, m_swapchain.getExtent());
	vkutil::makePresentable(currentCmd(), m_swapchain.getCurrentImage());

	VK_CHECK(vkEndCommandBuffer(currentCmd()));

	VkSemaphoreSubmitInfo waitInfo =
		vkinit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, getCurrentFrame()._swapchainSemaphore);
	VkSemaphoreSubmitInfo signalInfo =
		vkinit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, getCurrentFrame()._renderSemaphore);

	VkCommandBufferSubmitInfo cmdinfo = vkinit::commandBufferSubmitInfo(currentCmd());
	VkSubmitInfo2 submitInfo = vkinit::submitInfo(&cmdinfo, &signalInfo, &waitInfo);

	m_device.submitCommand(1, &submitInfo, getCurrentFrame()._renderFence);
	m_swapchain.presentOnScreen(getCurrentFrame()._renderSemaphore);

	m_frame++;
}

void VkEngine::drawGeometryTest() {
	// preparation
	VkCommandBuffer cmd = currentCmd();

	vkutil::makeColorWriteable(cmd, m_drawImage);
	VkRenderingAttachmentInfo colorAttachment = vkinit::attachmentInfo(m_drawImage.imageView, nullptr, VK_IMAGE_LAYOUT_GENERAL);
	VkRenderingInfo renderInfo = vkinit::renderingInfo(m_drawExtent, &colorAttachment, nullptr);

	vkCmdBeginRendering(cmd, &renderInfo);

	vkutil::setViewport(cmd, m_drawExtent);
	vkutil::setScissor(cmd, m_drawExtent);

	m_sceneData.sunlightColor = glm::vec4{1.f, 1.f, 1.f, 1.f};

	// global scene data
	AllocatedBuffer gpuSceneDataBuffer;
	VK_CHECK(m_device.createBuffer(sizeof(GPUSceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU,
								   &gpuSceneDataBuffer, true));

	VK_CHECK(m_device.fillBuffer(&gpuSceneDataBuffer, &m_sceneData, sizeof(GPUSceneData)));

	m_globalSceneDescriptor.writeBuffer(0, gpuSceneDataBuffer.buffer, sizeof(GPUSceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	VK_CHECK(m_device.allocateDescriptorSet(&m_globalSceneDescriptor, &getCurrentFrame()._descriptorAllocator, true));

	getCurrentFrame()._deletionQueue.push_function(
		[this, gpuSceneDataBuffer]() mutable { m_device.destroyBuffer(&gpuSceneDataBuffer); });

	// push constants for matrices and vertex buffer address
	GPUDrawPushConstants push_constants;
	push_constants.worldMatrix = glm::mat4{1.f};
	push_constants.vertexBuffer = m_testMesh.vertexBufferAddress;
	m_meshPipeline.pushConstants(cmd, &push_constants);

	m_meshPipeline.bind(cmd);

	vkCmdBindIndexBuffer(cmd, m_testMesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(cmd, 6, 1, 0, 0, 0);

	vkCmdEndRendering(cmd);
}

void VkEngine::drawComputeTest() {
	VkCommandBuffer cmd = currentCmd();

	vkutil::makeWriteable(cmd, m_drawImage);

	m_computePipeline.bind(cmd);

	vkCmdDispatch(cmd, m_drawImage.imageExtent.width, m_drawImage.imageExtent.height, 1);
}

void VkEngine::initTestData() {
	std::array<Vertex, 4> rect_vertices;

	rect_vertices[0].position = {0.5, -0.5, 0};
	rect_vertices[1].position = {0.5, 0.5, 0};
	rect_vertices[2].position = {-0.5, -0.5, 0};
	rect_vertices[3].position = {-0.5, 0.5, 0};

	rect_vertices[0].color = {0, 0, 0, 1};
	rect_vertices[1].color = {0.5, 0.5, 0.5, 1};
	rect_vertices[2].color = {1, 0, 0, 1};
	rect_vertices[3].color = {0, 1, 0, 1};

	std::array<uint32_t, 6> rect_indices = {0, 1, 2, 2, 1, 3};

	VK_CHECK(m_device.uploadMesh(&m_testMesh, rect_indices, rect_vertices));

	// textures
	uint32_t white = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));
	VK_CHECK(
		m_device.createFilledImage(&m_whiteTexture, &white, {1, 1, 1}, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT));

	uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
	uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 1));
	std::array<uint32_t, 16 * 16> pixels;

	for (int x = 0; x < 16; x++)
		for (int y = 0; y < 16; y++)
			pixels[y * 16 + x] = ((x % 2) ^ (y % 2)) ? magenta : black;

	VK_CHECK(m_device.createFilledImage(&m_checkboardTexture, pixels.data(), {16, 16, 1}, VK_FORMAT_R8G8B8A8_UNORM,
										VK_IMAGE_USAGE_SAMPLED_BIT));

	VkSamplerCreateInfo sampl = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = VK_FILTER_NEAREST,
		.minFilter = VK_FILTER_NEAREST,
	};

	VK_CHECK(m_device.createSampler(&m_defaultSamplerNearest, &sampl));

	sampl.magFilter = VK_FILTER_LINEAR;
	sampl.minFilter = VK_FILTER_LINEAR;
	VK_CHECK(m_device.createSampler(&m_defaultSamplerLinear, &sampl));
}

void VkEngine::destroy() {
	if (!m_initiliazed)
		return;

	m_device.waitIdle();

	for (uint32_t i = 0; i < FRAME_OVERLAP; i++) {
		m_frames[i]._deletionQueue.flush();
	}

	m_swapchain.destroy();

	m_device.destroy();

	m_window.destroy();
}
