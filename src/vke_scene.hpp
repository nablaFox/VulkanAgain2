#pragma once

#include "vke_system.hpp"

namespace vke {

class VkEngine;

class VkeScene {
	friend class VkeSceneManager;

public:
	void load(); // load scene resources

	entt::entity addEntity() { return m_entities.create(); }

	template <typename T, typename... Args>
	entt::entity addEntity(Args&&... args) {
		entt::entity entity = m_entities.create();
		m_entities.emplace<T>(entity, std::forward<Args>(args)...);
		return entity;
	}

	template <typename T, typename... Args>
	void addComponent(entt::entity entity, Args&&... component) {
		m_entities.emplace<T>(entity, std::forward<Args>(component)...);
	}

	void removeEntity(entt::entity entity);

	virtual void setup() = 0; // let the user setup the scene (add entities, etc)

protected:
	entt::registry m_entities;
	std::string name;
};

class VkeSceneManager {
public:
	VkeSceneManager(VkeSystemManager& systemManager) : systemManager(systemManager) {}

	void update(float deltaTime);

	void registerScene(std::string name, std::unique_ptr<VkeScene> scene);

	void switchScene(const std::string& name);

public:
	VkeScene& getCurrentScene() { return *currentScene; }

private:
	std::unordered_map<std::string, std::unique_ptr<VkeScene>> scenes;
	std::unique_ptr<VkeScene> currentScene;
	VkeSystemManager& systemManager;
};

template <typename T>
struct is_vke_scene {
	static constexpr bool value = std::is_base_of<VkeScene, T>::value;
};

} // namespace vke
