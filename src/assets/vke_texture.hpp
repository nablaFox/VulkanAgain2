#pragma once

#include "vke_asset.hpp"

namespace vke {

struct PixelData;

class VkeTexture : public VkeAsset {
	friend class VkeMaterial;

public:
	void loadFromFile(const std::string& filename);
	void setPixels();

private:
	std::vector<PixelData> pixels; // temporary
};

}; // namespace vke
