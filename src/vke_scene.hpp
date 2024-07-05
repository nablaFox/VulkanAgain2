#pragma once

#include <entt.hpp>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

#include "vke_asset.hpp"

namespace vke {

class VkEngine;

class VkeScene : public VkeAsset {
	friend class VkeSceneManager;
	friend class VkeSystemManager;

public:
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

protected:
	entt::registry m_entities;
};

class VkeSceneManager : public VkeAssetManager<VkeScene> {
	friend class VkeSystemManager;

public:
	void switchScene(const std::string& name);

	void switchScene(std::unique_ptr<VkeScene> scene);

private:
	std::shared_ptr<VkeScene> currentScene;
};

template <typename T>
using enable_if_vke_scene_t = std::enable_if_t<std::is_base_of_v<VkeScene, T>>;

} // namespace vke
