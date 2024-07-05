#include "vke_system.hpp"

using namespace vke;

void VkeSystemManager::updateAll(float deltaTime) {
	for (auto& system : systems)
		system->update(deltaTime);
}

void VkeSystemManager::awakeAll() {
	for (auto& system : systems) {
		system->awake();
	}
}
