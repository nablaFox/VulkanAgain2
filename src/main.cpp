#include "vke_engine.hpp"

int main(int argc, char* argv[]) {
	vke::VkEngine engine;

	engine.init();

	engine.run();

	engine.destroy();

	return 0;
}
