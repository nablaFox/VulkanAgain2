#include "vke_scene.hpp"

#include <fmt/core.h>

using namespace vke;

void VkeScene::removeEntity(entt::entity entity) { m_entities.destroy(entity); }

void VkeSceneManager::switchScene(std::unique_ptr<VkeScene> scene) { currentScene = std::move(scene); }

void VkeSceneManager::switchScene(const std::string& name) {
	if (currentScene != nullptr && currentScene->name == name) {
		fmt::println("Already in '{}' scene", name);
		return;
	}

	fmt::println("Switching to '{}' scene", name);
	currentScene = assets[name];
}
