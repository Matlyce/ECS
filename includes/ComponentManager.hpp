#pragma once

#include <unordered_map>
#include <typeindex>
#include <memory>
#include <iostream>
#include "AComponentStorage.hpp"
#include "ComponentStorage.hpp"
#include "Types.hpp"

/**
 * @class ComponentManager
 * @brief Manages component arrays and types.
 */
class ComponentManager
{
    public:
        /**
         * @brief Registers a component type.
         * @tparam T Component type.
         */
        template <typename T>
        void registerComponent()
        {
            std::type_index typeName = typeid(T);

            componentTypes.insert({typeName, nextComponentType});
            ComponentStorages.insert({typeName, std::make_shared<ComponentStorage<T>>()});
            printf("Registering component type %d - (%s)\n", nextComponentType, typeName.name());
            nextComponentType++;
        }

        /**
         * @brief Gets the component type ID for a component.
         * @tparam T Component type.
         * @return The component type ID.
         */
        template <typename T>
        ComponentTypeID getComponentTypeID()
        {
            std::type_index typeName = typeid(T);
            return componentTypes[typeName];
        }

        /**
         * @brief Adds a component to an entity.
         * @tparam T Component type.
         * @param entity The entity.
         * @param component The component data.
         */
        template <typename T>
        void addComponent(Entity entity, T component)
        {
            GetComponentStorage<T>()->insertData(entity, component);
        }

        /**
         * @brief Removes a component from an entity.
         * @tparam T Component type.
         * @param entity The entity.
         */
        template <typename T>
        void removeComponent(Entity entity)
        {
            GetComponentStorage<T>()->removeData(entity);
        }

        /**
         * @brief Gets a component from an entity.
         * @tparam T Component type.
         * @param entity The entity.
         * @return Reference to the component data.
         */
        template <typename T>
        T& getComponent(Entity entity)
        {
            return GetComponentStorage<T>()->getData(entity);
        }

        /**
         * @brief Called when an entity is destroyed.
         * @param entity The destroyed entity.
         */
        void entityDestroyed(Entity entity)
        {
            for (auto const& pair : ComponentStorages)
            {
                auto const& component = pair.second;
                component->entityDestroyed(entity);
            }
        }

    private:
        std::unordered_map<std::type_index, ComponentTypeID> componentTypes{};
        std::unordered_map<std::type_index, std::shared_ptr<AComponentStorage>> ComponentStorages{};
        ComponentTypeID nextComponentType{};

        /**
         * @brief Get the Component Storage object
         * @tparam T
         * @return std::shared_ptr<ComponentStorage<T>>
         */
        template <typename T>
        std::shared_ptr<ComponentStorage<T>> GetComponentStorage()
        {
            std::type_index typeName = typeid(T);
            return std::static_pointer_cast<ComponentStorage<T>>(ComponentStorages[typeName]);
        }
};
