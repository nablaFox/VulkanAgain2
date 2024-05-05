#include "vke_pipelines.hpp"
#include "vke_engine.hpp"

namespace vke {

void VkEngine::init(GameEngineSettings settings) {
	VkeWindow::create(m_window);
	VK_CHECK(m_window->init(settings.appName, settings.windowWidth, settings.windowHeight));

	VkeDevice::create(m_device);
	VK_CHECK(m_device->initialize(m_window));

	VK_CHECK(m_device->initFrameData(m_frames, FRAME_OVERLAP));

	m_initiliazed = true;
}

void VkEngine::initPipelines() {
	VK_CHECK(m_device->createShader(m_vertexShader, "shaders/vert.spv"));
	VK_CHECK(m_device->createShader(m_fragmentShader, "shaders/frag.spv"));

	m_meshPipeline.init()
		.setShaders(m_vertexShader, m_fragmentShader)
		.setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
		.setPolygonMode(VK_POLYGON_MODE_FILL)
		.setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE)
		.setMultisamplingNone()
		.disableBlending()
		.disableDepthTest()
		.setColorAttachmentFormat(m_device->getDrawContext().imageFormat);

	VK_CHECK(m_device->createGraphicsPipeline(m_meshPipeline));
}

// TEMP: this should be the entry point of the engine for the user code
void VkEngine::run() {
	bool quit = false;

	while (!quit) {
		glfwPollEvents();

		if (glfwWindowShouldClose(m_window->getWindow()))
			quit = true;

		startFrame();

		drawTest();

		endFrame();
	}
}

void VkEngine::startFrame() {
	VK_CHECK(vkWaitForFences(m_device->getDevice(), 1, &getCurrentFrame()._renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(m_device->getDevice(), 1, &getCurrentFrame()._renderFence));

	getCurrentFrame()._deletionQueue.flush();

	// 	m_swapchain.acquireImage(getCurrentFrame()._swapchainSemaphore);

	VK_CHECK(vkResetCommandBuffer(getCurrentFrame()._commandBuffer, 0));

	// 	m_drawExtent.width = m_drawImage.imageExtent.width;
	// 	m_drawExtent.height = m_drawImage.imageExtent.height;

	// 	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	// 	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));
	// 	vkutil::makeWriteable(cmd, m_drawImage);
}

void VkEngine::endFrame() {
	// 	vkutil::copyImageToImage(cmd, m_drawImage, m_swapchain.getCurrentImage(), m_drawExtent, m_swapchain.getExtent());
	// 	vkutil::makePresentable(cmd, m_swapchain.getCurrentImage());

	// 	VK_CHECK(vkEndCommandBuffer(cmd));
	//
	// 	VkSemaphoreSubmitInfo waitInfo =
	// 		vkinit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
	// getCurrentFrame()._swapchainSemaphore); 	VkSemaphoreSubmitInfo signalInfo =
	// 		vkinit::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, getCurrentFrame()._renderSemaphore);
	//
	// 	VkCommandBufferSubmitInfo cmdinfo = vkinit::commandBufferSubmitInfo(cmd);
	// 	VkSubmitInfo2 submitInfo = vkinit::submitInfo(&cmdinfo, &signalInfo, &waitInfo);
	//
	// 	VK_CHECK(vkQueueSubmit2(m_graphicsQueue, 1, &submitInfo, getCurrentFrame()._renderFence));
	//
	// 	m_swapchain.presentOnScreen(getCurrentFrame()._renderSemaphore);
	//
	// 	m_frame++;
}

void VkEngine::drawTest() {}

void VkEngine::destroy() {
	if (!m_initiliazed)
		return;

	m_device->waitIdle();

	m_device->destroy();

	m_window->destroy();
}

} // namespace vke
