#include "vke_scene.hpp"

#include <fmt/core.h>

using namespace vke;

void VkeScene::load() {
	fmt::println("Loading '{}' scene", name);
	//
}

void VkeScene::removeEntity(entt::entity entity) { m_entities.destroy(entity); }

void VkeSceneManager::update(float deltaTime) {
	if (!currentScene)
		return;

	systemManager.updateAll(deltaTime);
}

void VkeSceneManager::registerScene(std::string name, std::unique_ptr<VkeScene> scene) {
	scene->name = name;
	scenes[name] = std::move(scene);
}

void VkeSceneManager::switchScene(const std::string& name) {
	if (currentScene != nullptr && currentScene->name == name) {
		fmt::println("Already in '{}' scene", name);
		return;
	}

	fmt::println("Switching to '{}' scene", name);

	currentScene = std::move(scenes[name]);
	currentScene->load();
	currentScene->setup();

	systemManager.updateEntities(currentScene->m_entities);
}
