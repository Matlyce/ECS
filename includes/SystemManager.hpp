/**
 * @file SystemManager.hpp
 * @brief Manages registration and retrieval of systems in the ECS framework
 */
#pragma once

#include <unordered_map>
#include <typeindex>
#include <memory>
#include "System.hpp"
#include "Types.hpp"

/**
 * @class SystemManager
 * @brief Manages systems and their component signatures
 *
 * The SystemManager handles the registration, retrieval, and component signature
 * management for all systems in the ECS architecture. It ensures that entities
 * with matching component signatures are properly associated with relevant systems.
 */
class SystemManager
{
    public:
        /**
         * @brief Registers a new system
         * @tparam T System type to register
         * @return A shared pointer to the newly created system
         *
         * Creates a new system of type T and stores it internally, indexed by type.
         */
        template <typename T>
        std::shared_ptr<T> registerSystem()
        {
            std::type_index typeName = typeid(T);

            auto system = std::make_shared<T>();
            systems.insert({typeName, system});
            return system;
        }

        /**
         * @brief Retrieves an existing system
         * @tparam T System type to retrieve
         * @return A shared pointer to the requested system
         * @throws std::out_of_range if the system was not registered
         */
        template <typename T>
        std::shared_ptr<T> getSystem()
        {
            std::type_index typeName = typeid(T);
            return std::static_pointer_cast<T>(systems.at(typeName));
        }

        /**
         * @brief Sets the component signature for a system
         * @tparam T System type to set signature for
         * @param signature The component signature that defines entity requirements
         *
         * The signature determines which entities the system will process,
         * based on the components those entities possess.
         */
        template <typename T>
        void setSignature(Signature signature)
        {
            std::type_index typeName = typeid(T);
            signatures.insert({typeName, signature});
        }

        /**
         * @brief Notifies all systems that an entity has been destroyed
         * @param entity The ID of the destroyed entity
         *
         * Removes the entity from all systems' entity sets.
         */
        void entityDestroyed(Entity entity)
        {
            for (auto const &pair : systems)
            {
                auto const &system = pair.second;
                system->entities.erase(entity);
            }
        }

        /**
         * @brief Updates systems when an entity's component signature changes
         * @param entity The entity whose signature changed
         * @param entitySignature The entity's new component signature
         *
         * Adds the entity to systems whose signatures match the entity's new signature,
         * and removes it from systems that no longer match.
         */
        void entitySignatureChanged(Entity entity, Signature entitySignature)
        {
            for (auto const &pair : systems)
            {
                auto const &type = pair.first;
                auto const &system = pair.second;
                auto const &systemSignature = signatures[type];

                if ((entitySignature & systemSignature) == systemSignature)
                {
                    system->entities.insert(entity);
                }
                else
                {
                    system->entities.erase(entity);
                }
            }
        }

    private:
        /** Maps system types to their required component signatures */
        std::unordered_map<std::type_index, Signature> signatures{};

        /** Maps system types to their instances */
        std::unordered_map<std::type_index, std::shared_ptr<System>> systems{};
};
