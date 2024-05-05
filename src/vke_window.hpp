#pragma once

#include "vke_types.hpp"

namespace vke {

class VkeWindow {
public:
	VkeWindow(){};

	VkResult init(const char* title, uint32_t width, uint32_t height);
	void destroy();

	GLFWwindow* getWindow() { return m_window; }

	VkExtent2D getExtent() { return m_windowExtent; }

private:
	VkExtent2D m_windowExtent{700, 800};
	GLFWwindow* m_window;
};

} // namespace vke
