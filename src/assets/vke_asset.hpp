#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace vke {

template <typename AssetType>
class VkeAssetManager {
public:
	template <typename T>
	std::shared_ptr<T> registerAsset(const std::string& name) {
		auto asset = std::make_shared<T>();
		asset->setup();
		asset->bootstrap();
		asset->name = name;
		assets[name] = std::move(asset);
		return std::static_pointer_cast<T>(assets[name]);
	}

	std::shared_ptr<AssetType> registerAsset(const std::string& name) { return registerAsset<AssetType>(name); }

	void unregisterAsset(const std::string& name) { assets.erase(name); }

	virtual ~VkeAssetManager() = default;

protected:
	std::unordered_map<std::string, std::shared_ptr<AssetType>> assets;
};

class VkeAsset {
	template <typename AssetType>
	friend class VkeAssetManager;

public:
	std::string name;

protected:
	virtual void setup(){};		// for configuring the asset
	virtual void bootstrap(){}; // for loading the asset, if necessary
};

} // namespace vke
