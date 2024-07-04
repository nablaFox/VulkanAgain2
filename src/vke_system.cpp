#include "vke_system.hpp"
#include "vke_engine.hpp"

using namespace vke;

void VkeSystemManager::registerSystem(std::unique_ptr<VkeSystem> system, VkEngine* engine) {
	system->m_engine = engine;
	systems.push_back(std::move(system));
}

void VkeSystemManager::updateAll(float deltaTime) {
	for (auto& system : systems)
		system->update(deltaTime);
}

void VkeSystemManager::updateEntities(entt::registry& entities) {
	for (auto& system : systems)
		system->m_entities = &entities;
}
