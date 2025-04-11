#pragma once

#include <unordered_map>
#include "AComponentStorage.hpp"
#include "Types.hpp"

/**
 * @class ComponentStorage
 * @brief Stores component data for entities using an unordered map.
 * @tparam T Component type.
 */
template <typename T>
class ComponentStorage : public AComponentStorage {
public:
    /**
     * @brief Inserts or updates component data for an entity.
     * @param entity The entity.
     * @param component The component data.
     */
    void insertData(Entity entity, T component) {
        componentMap.emplace(entity, component);
    }

    /**
     * @brief Removes component data for an entity.
     * @param entity The entity.
     */
    void removeData(Entity entity) {
        componentMap.erase(entity);
    }

    /**
     * @brief Retrieves component data for an entity.
     * @param entity The entity.
     * @return Reference to the component data.
     * @throws std::out_of_range if the entity does not exist.
     */
    T& getData(Entity entity) {
        return componentMap.at(entity);
    }

    /**
     * @brief Checks if an entity has a component.
     * @param entity The entity.
     * @return True if the entity has a component, otherwise false.
     */
    bool hasData(Entity entity) const {
        return componentMap.find(entity) != componentMap.end();
    }

    /**
     * @brief Called when an entity is destroyed.
     * @param entity The destroyed entity.
     */
    void entityDestroyed(Entity entity) override {
        componentMap.erase(entity);
    }

private:
    std::unordered_map<Entity, T> componentMap;
};
