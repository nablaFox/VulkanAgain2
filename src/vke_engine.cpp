#include "vke_pipelines.hpp"
#include "vke_engine.hpp"
#include "vke_images.hpp"
#include "vke_initializers.hpp"

namespace vke {

void VkEngine::init(GameEngineSettings settings) {
	VK_CHECK(m_window.init(settings.appName, settings.windowWidth, settings.windowHeight));

	VK_CHECK(m_device.init(&m_window));

	m_swapchain.init(&m_device, m_window.getExtent(), VK_FORMAT_B8G8R8A8_UNORM);

	VK_CHECK(m_device.initFrameData(m_frames, FRAME_OVERLAP));

	VK_CHECK(m_device.createCommandPool(&m_immData._commandPool, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT));
	VK_CHECK(m_device.allocateCommandBuffer(&m_immData._commandBuffer, m_immData._commandPool));
	VK_CHECK(m_device.createFence(&m_immData._fence, VK_FENCE_CREATE_SIGNALED_BIT));

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

	m_drawImageDescriptorSet.bindImage(m_drawImage.imageView, nullptr, VK_IMAGE_LAYOUT_GENERAL, VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHECK(m_device.allocateDescriptorSet(&m_drawImageDescriptorSet));

	auto bufferRange = vkutil::getPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(GPUDrawPushConstants));

	m_meshPipeline.setShaders(m_vertexShader, m_fragmentShader)
		.setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
		.setPolygonMode(VK_POLYGON_MODE_FILL)
		.setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE)
		.setMultisamplingNone()
		.disableBlending()
		.disableDepthTest()
		.setColorAttachmentFormat(m_drawImage.imageFormat)
		.setPushConstantRange(bufferRange);

	m_computePipeline.setShader(m_computeShader).setDescriptorSet(m_drawImageDescriptorSet);

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

		// drawComputeTest();
		drawGeometryTest();

		endFrame();
	}
}

void VkEngine::startFrame() {
	VK_CHECK(vkWaitForFences(m_device.getDevice(), 1, &getCurrentFrame()._renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(m_device.getDevice(), 1, &getCurrentFrame()._renderFence));

	getCurrentFrame()._deletionQueue.flush();

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

	// draw
	GPUDrawPushConstants push_constants;
	push_constants.worldMatrix = glm::mat4{1.f};
	push_constants.vertexBuffer = m_testMesh.vertexBufferAddress;

	m_meshPipeline.bind(cmd);
	m_meshPipeline.pushConstants(cmd, &push_constants);
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

	m_testMesh = uploadMesh(rect_indices, rect_vertices);
}

void VkEngine::immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function) {
	VkCommandBuffer cmd = m_immData._commandBuffer;

	VK_CHECK(vkResetFences(m_device.getDevice(), 1, &m_immData._fence));
	VK_CHECK(vkResetCommandBuffer(cmd, 0));

	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	function(cmd);

	VK_CHECK(vkEndCommandBuffer(cmd));

	VkCommandBufferSubmitInfo cmdinfo = vkinit::commandBufferSubmitInfo(cmd);
	VkSubmitInfo2 submit = vkinit::submitInfo(&cmdinfo, nullptr, nullptr);

	VK_CHECK(m_device.submitCommand(1, &submit, m_immData._fence));
	VK_CHECK(vkWaitForFences(m_device.getDevice(), 1, &m_immData._fence, true, 9999999999));
}

GPUMeshBuffers VkEngine::uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices) {
	const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
	const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

	GPUMeshBuffers newSurface;
	VK_CHECK(m_device.createBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
								   VMA_MEMORY_USAGE_GPU_ONLY, &newSurface.indexBuffer));

	VK_CHECK(m_device.createBuffer(vertexBufferSize,
								   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
									   VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
								   VMA_MEMORY_USAGE_GPU_ONLY, &newSurface.vertexBuffer));

	VkBufferDeviceAddressInfo deviceAdressInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = newSurface.vertexBuffer.buffer,
	};

	newSurface.vertexBufferAddress = vkGetBufferDeviceAddress(m_device.getDevice(), &deviceAdressInfo);

	AllocatedBuffer staging;
	void* data;

	VK_CHECK(m_device.createStagingBuffer(vertexBufferSize + indexBufferSize, &staging, data));

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

	VK_CHECK(m_device.destroyBuffer(&staging));

	return newSurface;
}

void VkEngine::destroy() {
	if (!m_initiliazed)
		return;

	m_device.waitIdle();

	m_swapchain.destroy();

	m_device.destroy();

	m_window.destroy();
}

} // namespace vke
