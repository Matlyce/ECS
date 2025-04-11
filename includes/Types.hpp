#pragma once

#include <cstdint>
#include <bitset>

using Entity = std::uint32_t;
const Entity MAX_ENTITIES = 5000;

using ComponentTypeID = std::uint8_t;
const ComponentTypeID MAX_COMPONENTS = 64;

using Signature = std::bitset<MAX_COMPONENTS>;
