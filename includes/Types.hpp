/**
 * @file Types.hpp
 * @brief Core type definitions for the ECS framework
 */
#pragma once

#include <cstdint>
#include <bitset>

/**
 * @typedef Entity
 * @brief Entity identifier type
 *
 * Represents a unique identifier for an entity in the ECS system.
 * Implemented as a 32-bit unsigned integer for efficiency.
 */
using Entity = std::uint32_t;

/**
 * @var MAX_ENTITIES
 * @brief Maximum number of entities allowed in the system
 */
const Entity MAX_ENTITIES = 5000;

/**
 * @typedef ComponentTypeID
 * @brief Component type identifier
 *
 * Used to uniquely identify different component types.
 * Implemented as an 8-bit unsigned integer, limiting the system to 256 component types.
 */
using ComponentTypeID = std::uint8_t;

/**
 * @var MAX_COMPONENTS
 * @brief Maximum number of component types allowed in the system
 *
 * Limited to 64 to match the size of the signature bitset.
 */
const ComponentTypeID MAX_COMPONENTS = 64;

/**
 * @typedef Signature
 * @brief Component signature for entities
 *
 * A bitset where each bit represents whether an entity has a specific component type.
 * Used for efficient component membership testing and system-entity matching.
 */
using Signature = std::bitset<MAX_COMPONENTS>;
