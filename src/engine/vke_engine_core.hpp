#pragma once

#include "../renderer/vke_descriptors.hpp"
#include "../renderer/vke_device.hpp"
#include "../assets/vke_material.hpp"
#include "../renderer/vke_types.hpp"
#include "../renderer/vke_utils.hpp"
#include "../renderer/vke_window.hpp"
#include "../renderer/vke_pipelines.hpp"
#include "../assets/vke_scene.hpp"
#include "../systems/vke_system_manager.hpp"

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

	std::shared_ptr<VkeScene> createScene(const std::string& name) { return m_sceneManager.registerAsset(name); }
	VkeScene& getCurrentScene() { return m_sceneManager.getCurrentScene(); }
	void switchScene(const std::string& name) { m_sceneManager.switchScene(name); }

	template <typename T>
	struct dependent_false : std::false_type {};

	template <typename T>
	void registerSystem() {
		static_assert(std::is_base_of<VkeSystem, T>::value, "T must be a VkeSystem derived class");
		m_systemManager.registerSystem<T>(this);
	}

	template <typename T>
	void registerAsset(const std::string& name) {
		static_assert(std::is_base_of<VkeAsset, T>::value, "T must be a VkeAsset derived class");

		if constexpr (std::is_base_of<VkeScene, T>::value) {
			m_sceneManager.registerAsset<T>(name);
		} else if constexpr (std::is_base_of<VkeMaterial, T>::value) {
			m_materialManager.registerAsset<T>(name);
		} else {
			static_assert(dependent_false<T>::value, "T must be a VkeAsset derived class");
		}
	}

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
	VkeSystemManager m_systemManager;
	VkeMaterialManager m_materialManager;

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
