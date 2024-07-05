#include "vke_system.hpp"
#include "vke_engine.hpp"

using namespace vke;

void VkeSystemManager::updateAll(float deltaTime) {
	for (auto& system : systems)
		system->update(deltaTime);
}

void VkeSystemManager::awakeAll() {
	for (auto& system : systems) {
		system->m_entities = &m_sceneManager.currentScene->m_entities;
		system->awake();
	}
}
