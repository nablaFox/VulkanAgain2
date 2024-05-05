#pragma once

#include "vke_types.hpp"

namespace vke {

class VkeDevice;

class VkeAllocator {
public:
	VkeAllocator(VkeDevice* device) : m_device(device) {}
	~VkeAllocator();

	void init();
	void destroy();

	void createDrawImage(VkExtent2D extent, AllocatedImage* image);
	void destroyImage(AllocatedImage& image);

private:
	VmaAllocator m_allocator;
	VkeDevice* m_device;
};

} // namespace vke
