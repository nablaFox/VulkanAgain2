#pragma once

#include "vke_scene.hpp"

namespace vke {

class VkEngine;

class VkeSystem {
	friend class VkeSystemManager;

public:
	virtual void update(float deltaTime) = 0;
	virtual void awake() {}
	virtual void sleep() {}
	virtual void windowResized() {}

public:
	template <typename T>
	auto getEntities() {
		return m_entities->view<T>();
	}

protected:
	entt::registry* m_entities;
	VkEngine* m_engine;
};

class VkeSystemManager {
public:
	VkeSystemManager(VkeSceneManager& sceneManager) : m_sceneManager(sceneManager) {}

	template <typename T>
	void registerSystem(VkEngine* engine) {
		auto system = std::make_unique<T>();
		system->m_engine = engine;
		systems.push_back(std::move(system));
	}

	void updateAll(float deltaTime);
	void awakeAll();

private:
	std::vector<std::unique_ptr<VkeSystem>> systems;
	VkeSceneManager& m_sceneManager;
};

template <typename T>
using enable_if_vke_sys_t = std::enable_if_t<std::is_base_of_v<VkeSystem, T>>;

} // namespace vke
