#pragma once

#include <vector>
#include <memory>

namespace vke {

class VkeSystem;
class VkEngine;

class VkeSystemManager {
public:
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
};

} // namespace vke
