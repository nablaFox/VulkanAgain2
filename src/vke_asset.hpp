#pragma once

#include "vke_types.hpp"
#include <memory>

namespace vke {

class VkeAsset {
public:
	virtual void setup() = 0;
	std::string name;
};

template <typename AssetType>
class VkeAssetManager {
public:
	template <typename T>
	void registerAsset(const std::string& name) {
		setupAsset<T>(name);
	}

	void unregisterAsset(const std::string& name) { assets.erase(name); }

	virtual ~VkeAssetManager() = default;

protected:
	std::unordered_map<std::string, std::shared_ptr<AssetType>> assets;

	template <typename T>
	void setupAsset(const std::string& name) {
		auto asset = std::make_shared<T>();
		asset->setup();
		asset->name = name;
		assets[name] = std::move(asset);
	}
};

} // namespace vke
