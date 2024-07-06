#include "engine/vke_engine.hpp"

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

class CustomMaterial : public VkeMaterial {
public:
	void setup() override { fmt::print("Material setup\n"); };
};

class Scene2 : public VkeScene {
public:
	void setup() override { addEntity<DemoComponent>(20); }
};

class DemoSystem : public VkeSystem {
public:
	void awake() override {
		// auto scene = m_engine->createScene("test");
		// scene->addEntity<DemoComponent>(100);
		//

		// fmt::print("DemoSystem awake\n");
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
		registerAsset<CustomMaterial>("custom_material");

		registerSystem<DemoSystem>();

		switchScene("initial");
	}
};
