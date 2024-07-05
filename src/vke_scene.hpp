#pragma once

#include <entt.hpp>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

#include "vke_asset.hpp"

namespace vke {

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

	template <typename T>
	auto getEntities() {
		auto entities = m_entities.view<T>();
		return entities;
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

	VkeScene& getCurrentScene() { return *currentScene; }

private:
	std::shared_ptr<VkeScene> currentScene;
};

} // namespace vke
