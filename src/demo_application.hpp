#include "vke_engine.hpp"
#include "vke_system.hpp"

#include <iostream>

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
		auto scene = m_engine->createScene();
		scene->addEntity<DemoComponent>(100);

		m_engine->registerAsset("test", scene);

		fmt::print("DemoSystem awake\n");
	}

	void update(float deltaTime) override {
		int i = 0;

		currentScene().getEntities<DemoComponent>().each([this, &i](auto& component) {
			fmt::print("Entity {} DemoComponent value: {}\n", i, component.value1);
			component.value1++;

			if (component.value1 % 10 == 0) {
				// m_engine->switchScene("test");
			} else {
			}

			i++;
		});
	}
};

class DemoApplication : public Application {
public:
	void setup() override {
		registerAsset<InitialScene>("initial");
		registerAsset<Scene2>("scene2");

		registerSystem<DemoSystem>();

		switchScene("initial");
	}
};
