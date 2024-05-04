#pragma once

#include "vke_types.hpp"

namespace vke {

class VkEngine;

class VkeAllocator {
public:
	VkeAllocator(VkEngine* engine) : m_engine(engine) {}
	~VkeAllocator();

	void init();
	void destroy();

	void createDrawImage(VkExtent2D extent, AllocatedImage* image);
	void destroyImage(AllocatedImage& image);

private:
	VmaAllocator m_allocator;
	VkEngine* m_engine;
};

} // namespace vke
