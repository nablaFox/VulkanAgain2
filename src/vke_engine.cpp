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
	VK_CHECK(m_device.createDrawImage(m_window.getExtent(), &m_drawImage));

	m_initiliazed = true;
}

void VkEngine::initPipelines() {
	VK_CHECK(m_device.createShader(m_vertexShader, "shaders/vert.spv"));
	VK_CHECK(m_device.createShader(m_fragmentShader, "shaders/frag.spv"));

	m_meshPipeline.init()
		.setShaders(m_vertexShader, m_fragmentShader)
		.setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
		.setPolygonMode(VK_POLYGON_MODE_FILL)
		.setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE)
		.setMultisamplingNone()
		.disableBlending()
		.disableDepthTest()
		.setColorAttachmentFormat(m_drawImage.imageFormat);

	VK_CHECK(m_device.createGraphicsPipeline(m_meshPipeline));
}

// TEMP: this should be the entry point of the engine for the user code
void VkEngine::run() {
	bool quit = false;

	while (!quit) {
		glfwPollEvents();

		if (glfwWindowShouldClose(m_window.getWindow()))
			quit = true;

		startFrame();

		drawTest();

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
	vkutil::makeWriteable(currentCmd(), m_drawImage);
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

void VkEngine::drawTest() {}

void VkEngine::destroy() {
	if (!m_initiliazed)
		return;

	m_device.waitIdle();

	m_swapchain.destroy();

	m_device.destroy();

	m_window.destroy();
}

} // namespace vke
