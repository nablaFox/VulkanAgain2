#pragma once

#include <cstdint>
#include <bitset>

using Entity = uint32_t;
constexpr uint32_t MAX_ENTITIES = 10000;

using Component = uint32_t;
constexpr uint32_t MAX_COMPONENTS = 32;

using Signature = std::bitset<MAX_COMPONENTS>;
