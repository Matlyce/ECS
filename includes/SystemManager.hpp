#pragma once

#include <unordered_map>
#include <typeindex>
#include <memory>
#include "System.hpp"
#include "Types.hpp"

/**
 * @class SystemManager
 * @brief Manages systems and their signatures.
 */
class SystemManager
{
    public:
        /**
         * @brief Registers a new system.
         * @tparam T System type.
         * @return A shared pointer to the new system.
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
         * @brief Gets an existing system.
         * @tparam T System type.
         * @return A shared pointer to the system.
         */
        template <typename T>
        std::shared_ptr<T> getSystem()
        {
            std::type_index typeName = typeid(T);
            return std::static_pointer_cast<T>(systems.at(typeName));
        }

        /**
         * @brief Sets the signature for a system.
         * @tparam T System type.
         * @param signature The signature to set.
         */
        template <typename T>
        void setSignature(Signature signature)
        {
            std::type_index typeName = typeid(T);
            signatures.insert({typeName, signature});
        }

        /**
         * @brief Notifies systems that an entity has been destroyed.
         * @param entity The destroyed entity.
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
         * @brief Updates systems when an entity's signature changes.
         * @param entity The entity.
         * @param entitySignature The new signature.
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
        std::unordered_map<std::type_index, Signature> signatures{};
        std::unordered_map<std::type_index, std::shared_ptr<System>> systems{};
};
