#pragma once

#include "vke_types.hpp"
#include <memory>

namespace vke {

class VkeAsset {
public:
	virtual void setup(){};
	std::string name;
};

template <typename AssetType>
class VkeAssetManager {
public:
	template <typename T>
	void registerAsset(const std::string& name) {
		setupAsset<T>(name);
	}

	virtual std::shared_ptr<AssetType> createAsset() {
		auto asset = std::make_shared<AssetType>();
		asset->setup();
		return asset;
	}

	void registerAsset(const std::string& name, std::shared_ptr<AssetType>& asset) {
		assets[name] = asset;
		assets[name]->name = name;
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
