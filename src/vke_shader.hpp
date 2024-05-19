#pragma once

#include "vke_types.hpp"

namespace vke {

class VkeShader {
	friend class VkeDevice;

public:
	enum ShaderType {
		VERTEX,
		FRAGMENT,
		COMPUTE,
	};

	VkeShader() {}

	const VkShaderModule& getModule() { return m_shaderModule; }

private:
	VkShaderModule m_shaderModule;
	const char* m_name;

	VkResult loadShaderModule(const char* filePath, std::vector<uint32_t>& buffer);
};

} // namespace vke
