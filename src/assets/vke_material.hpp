#pragma once

#include "vke_asset.hpp"
#include "vke_texture.hpp"
#include "../renderer/vke_pipelines.hpp"

namespace vke {

class VkeMaterial : public VkeAsset {

public:
	struct RenderingConfig {};

	VkeGraphicsPipeline* pipeline;

	std::string vertexShader;
	std::string fragShader;

	RenderingConfig renderingConfig;

	std::shared_ptr<VkeTexture> texture; // we can have multiple textures

	void bootstrap() override final { fmt::print("Bootstrapping material\n"); }
};

class VkeMaterialManager : public VkeAssetManager<VkeMaterial> {};

}; // namespace vke
