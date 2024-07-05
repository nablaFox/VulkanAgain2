#pragma once

#include "vke_scene.hpp"
#include "vke_engine.hpp"

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
	VkeScene& currentScene() { return m_engine->getCurrentScene(); }

protected: // TODO: make this private
	VkEngine* m_engine;
};

// for other type of systems, which includes particular methods
class VkeSystemScript : public VkeSystem {};

} // namespace vke
