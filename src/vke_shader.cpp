#include "vke_shader.hpp"
#include <fstream>

using namespace vke;

VkResult VkeShader::loadShaderModule(const char* filePath, std::vector<uint32_t>& buffer) {
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		return VK_ERROR_INITIALIZATION_FAILED;

	size_t fileSize = (size_t)file.tellg();

	buffer.resize(fileSize / sizeof(uint32_t));

	file.seekg(0);

	file.read((char*)buffer.data(), fileSize);

	file.close();

	return VK_SUCCESS;
}
