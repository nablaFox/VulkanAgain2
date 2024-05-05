#pragma once

#include "vke_frame.hpp"
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

class VkEngine {
public:
	static constexpr GameEngineSettings defaultSettings{};

	bool m_initiliazed{false};
	int m_frame{0};

	static constexpr unsigned int FRAME_OVERLAP = 2;

	void init(GameEngineSettings settings = defaultSettings);
	void run();
	void destroy();

	void drawTest();

private:
	VkeWindow* m_window;
	VkeSwapchain* m_swapchain;
	VkeDevice* m_device;

	FrameData m_frames[FRAME_OVERLAP];
	FrameData& getCurrentFrame() { return m_frames[m_frame % FRAME_OVERLAP]; }

	VkeGraphicsPipeline m_meshPipeline;
	VkeComputePipeline m_computePipeline;

	VkeShader m_vertexShader;
	VkeShader m_fragmentShader;
	VkeShader m_computeShader;

private:
	void startFrame();
	void endFrame();

	void initPipelines();
};

} // namespace vke
