#pragma once

#include <entt.hpp>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

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
	void registerSystem(std::unique_ptr<VkeSystem> system, VkEngine* engine);

	void updateAll(float deltaTime);

	void updateEntities(entt::registry& entities);

private:
	std::vector<std::unique_ptr<VkeSystem>> systems;
};

template <typename T>
struct is_vke_system {
	static constexpr bool value = std::is_base_of<VkeSystem, T>::value;
};

} // namespace vke
