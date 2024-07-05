#pragma once

#include "vke_descriptors.hpp"
#include "vke_device.hpp"
#include "vke_types.hpp"
#include "vke_utils.hpp"
#include "vke_window.hpp"
#include "vke_pipelines.hpp"

#include "vke_system.hpp"
#include "vke_scene.hpp"

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

	VkeDescriptorAllocator _descriptorAllocator; // TODO: make growable
};

class VkEngine {
	friend class Application;

public:
	static constexpr GameEngineSettings defaultSettings{};

	bool m_initiliazed{false};
	int m_frame{0};

	static constexpr unsigned int FRAME_OVERLAP = 2;

	void init(GameEngineSettings settings = defaultSettings);
	void run();
	void destroy();

	void drawGeometryTest();
	void drawComputeTest();
	void initTestData();

	template <typename T>
	enable_if_vke_sys_t<T> registerSystem() {
		m_systemManager.registerSystem<T>(this);
	}

	template <typename T>
	enable_if_vke_scene_t<T> registerScene(const std::string& name) {
		m_sceneManager.registerAsset<T>(name);
	}

	void switchScene(const std::string& name) { m_sceneManager.switchScene(name); }

	GPUMeshBuffers m_testMesh;
	AllocatedImage m_whiteTexture;
	AllocatedImage m_checkboardTexture;
	VkSampler m_defaultSamplerLinear;
	VkSampler m_defaultSamplerNearest;

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

	GPUSceneData m_sceneData;

	VkeDescriptorAllocator m_globalDescriptorAllocator;
	VkeDescriptor m_drawImageDescriptor;
	VkeDescriptor m_globalSceneDescriptor;
	VkeDescriptor m_materialDescriptor;

	VkeSceneManager m_sceneManager;
	VkeSystemManager m_systemManager{m_sceneManager};

private:
	void startFrame();
	void endFrame();

	void initPipelines();
};

// TEMP: find a better way to define multiple projects/applications
class Application : public VkEngine {
public:
	virtual void setup() = 0;

	void run() { VkEngine::run(); }

	void init(GameEngineSettings settings = VkEngine::defaultSettings) {
		VkEngine::init(settings);
		setup();
		m_systemManager.awakeAll();
	}

	void destroy() { VkEngine::destroy(); }
};

} // namespace vke
