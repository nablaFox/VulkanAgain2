#pragma once

#include "vke_descriptors.hpp"
#include "vke_device.hpp"
#include "vke_types.hpp"
#include "vke_utils.hpp"
#include "vke_window.hpp"
#include "vke_pipelines.hpp"

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
	typename std::enable_if<is_vke_system<T>::value>::type registerSystem() {
		m_systemManager.registerSystem(std::make_unique<T>(), this);
	}

	template <typename T>
	typename std::enable_if<is_vke_scene<T>::value>::type registerScene(const std::string& name) {
		m_sceneManager.registerScene(name, std::make_unique<T>());
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

	VkeSystemManager m_systemManager;
	VkeSceneManager m_sceneManager{m_systemManager};

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
	}

	void destroy() { VkEngine::destroy(); }
};

} // namespace vke
