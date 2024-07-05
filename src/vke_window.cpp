#include "vke_window.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

using namespace vke;

// TODO: add other parameters
VkResult VkeWindow::init(const char* title, uint32_t width, uint32_t height) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);

	return m_window == nullptr ? VK_ERROR_INITIALIZATION_FAILED : VK_SUCCESS;
}

void VkeWindow::destroy() {
	glfwDestroyWindow(m_window);
	glfwTerminate();
}
