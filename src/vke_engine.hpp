#pragma once

#include "vke_device.hpp"
#include "vke_types.hpp"
#include "vke_utils.hpp"
#include "vke_window.hpp"
#include "vke_pipelines.hpp"

namespace vke {

struct GameEngineSettings {
	const char* appName = "Vulkan Engine";
	uint32_t windowWidth = 1280;
	uint32_t windowHeight = 720;
	bool resizableWindow = false;
};

struct FrameData {
	VkSemaphore _swapchainSemaphore, _renderSemaphore;
	VkFence _renderFence;

	VkCommandPool _commandPool;
	VkCommandBuffer _commandBuffer;

	vkutil::DeletionQueue _deletionQueue;
};

struct ImmediateData {
	VkCommandBuffer _commandBuffer;
	VkCommandPool _commandPool;
	VkFence _fence;
};

class VkEngine {
public:
	static constexpr GameEngineSettings defaultSettings{};

	bool m_initiliazed{false};
	int m_frame{0};

	static constexpr unsigned int FRAME_OVERLAP = 2;

	void init(GameEngineSettings settings = defaultSettings);
	void run();
	void destroy();

	void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
	GPUMeshBuffers uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices);

	void drawGeometryTest();
	void drawComputeTest();
	void initTestData();
	GPUMeshBuffers m_testMesh;

private:
	VkeWindow m_window;
	VkeDevice m_device;
	VkeSwapchain m_swapchain;

	AllocatedImage m_drawImage;
	VkExtent2D m_drawExtent;

	FrameData m_frames[FRAME_OVERLAP];
	FrameData& getCurrentFrame() { return m_frames[m_frame % FRAME_OVERLAP]; }
	VkCommandBuffer& currentCmd() { return getCurrentFrame()._commandBuffer; }

	VkeGraphicsPipeline m_meshPipeline;
	VkeComputePipeline m_computePipeline;

	VkeShader m_vertexShader;
	VkeShader m_fragmentShader;
	VkeShader m_computeShader;

	ImmediateData m_immData;

private:
	void startFrame();
	void endFrame();

	void initPipelines();
};

} // namespace vke
