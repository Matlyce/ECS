#pragma once

#include "Types.hpp"

/**
 * @class AComponentStorage
 * @brief Abstract base class for component arrays.
 */
class AComponentStorage
{
    public:
        virtual ~AComponentStorage() = default;

        /**
         * @brief Removes component data associated with an entity.
         * @param entity The entity to remove.
         */
        virtual void entityDestroyed(Entity entity) = 0;
};
