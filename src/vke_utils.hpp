#pragma once

#include "vke_types.hpp"

namespace vkutil {

struct DeletionQueue {
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()>&& deletor) { deletors.push_back(deletor); }

	void flush() {
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++)
			(*it)();

		deletors.clear();
	}
};

} // namespace vkutil
