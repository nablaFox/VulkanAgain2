#include "vke_engine.hpp"

using namespace vke;

struct DemoComponent {
	int value1;
	int value2;
};

struct DemoComponent2 {
	int value1;
	int value2;
};

class InitialScene : public VkeScene {
public:
	void setup() override {
		addEntity<DemoComponent>(5);
		addEntity<DemoComponent>(10);

		auto testEntity = addEntity();
		addComponent<DemoComponent>(testEntity, 5);

		auto entity = addEntity<DemoComponent>(-1000, 7);
		addComponent<DemoComponent2>(entity, 10, 20);
	}
};

class Scene2 : public VkeScene {
public:
	void setup() override { addEntity<DemoComponent>(20); }
};

class DemoSystem : public VkeSystem {
public:
	void awake() override {
		// setup entities
	}

	void update(float deltaTime) override {
		auto view = getEntities<DemoComponent>();

		int i = 0;
		for (auto entity : view) {
			auto& component = view.get<DemoComponent>(entity);
			component.value1++;
			fmt::print("Entity {} DemoComponent value: {}\n", i, component.value1);

			if (component.value1 > 100) {
				m_engine->switchScene("scene2");
			}

			i++;
		}
	}
};

class DemoApplication : public Application {
public:
	void setup() override {
		registerScene<InitialScene>("initial");
		registerScene<Scene2>("scene2");
		registerSystem<DemoSystem>();

		switchScene("initial");
	}
};
