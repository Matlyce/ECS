/**
 * @file Coordinator.hpp
 * @brief Central coordinator for the Entity Component System
 */
#pragma once

// Standard library includes for concurrency, memory management, and data structures
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <vector>

// Project-specific includes for core ECS components
#include "ComponentManager.hpp"
#include "EntityManager.hpp"
#include "SystemManager.hpp"
#include "Types.hpp"

/**
 * @class Coordinator
 * @brief Coordinates entities, components, and systems.
 *
 * This version defers modifications (destroyEntity, addComponent, removeComponent)
 * via a thread-safe command queue. Modifications are enqueued (async) and applied
 * when flushCommands() is called (typically once per frame).
 *
 * For cases where you require immediate (synchronous) execution you have two options:
 *   - Call the alternative sync methods (e.g. addComponentSync).
 *   - Turn off async modifications globally by calling setAsyncModifications(false).
 *
 * Note on getComponent: This method returns a reference for direct read/edit access.
 * If you plan to modify a component from multiple threads, you should either perform
 * the modification only after flushCommands() or provide a dedicated update method
 * that enqueues modifications.
 */
class Coordinator {
public:
    /**
     * @brief Initializes the coordinator
     *
     * Creates instances of the component, entity, and system managers.
     */
    void init() {
        componentManager = std::make_unique<ComponentManager>();
        entityManager = std::make_unique<EntityManager>();
        systemManager = std::make_unique<SystemManager>();
    }

    /**
     * @brief Creates a new entity
     * @return Unique identifier for the created entity
     *
     * Thread-safe creation of a new entity.
     */
    Entity createEntity() {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        return entityManager->createEntity();
    }

    /**
     * @brief Checks if an entity exists
     * @param entity Entity ID to check
     * @return True if the entity exists, false otherwise
     */
    bool entityExists(Entity entity) {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        return entityManager->entityExists(entity);
    }

    /**
     * @brief Destroys an entity and removes all its components
     * @param entity Entity ID to destroy
     *
     * Destroys the entity in a synchronized manner across all managers.
     */
    void destroyEntity(Entity entity) {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        entityManager->destroyEntity(entity);
        componentManager->entityDestroyed(entity);
        systemManager->entityDestroyed(entity);
    }

    /**
     * @brief Synchronously destroys an entity
     * @param entity Entity ID to destroy
     *
     * Alternative synchronous version of destroyEntity.
     * @note Currently identical to destroyEntity as both are synchronous
     */
    void destroyEntitySync(Entity entity) {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        entityManager->destroyEntity(entity);
        componentManager->entityDestroyed(entity);
        systemManager->entityDestroyed(entity);
    }

    /**
     * @brief Registers a new component type
     * @tparam T Component type to register
     */
    template <typename T>
    void registerComponent() {
        componentManager->registerComponent<T>();
    }

    /**
     * @brief Registers a new system
     * @tparam T System type to register
     * @return Shared pointer to the registered system
     */
    template <typename T>
    std::shared_ptr<T> registerSystem() {
        return systemManager->registerSystem<T>();
    }

    /**
     * @brief Gets a system by type
     * @tparam T System type to retrieve
     * @return Shared pointer to the requested system
     */
    template <typename T>
    std::shared_ptr<T> getSystem() {
        return systemManager->getSystem<T>();
    }

    /**
     * @brief Adds a component to an entity
     * @tparam T Component type to add
     * @param entity Entity to add the component to
     * @param component Component instance to add
     */
    template <typename T>
    void addComponent(Entity entity, T component) {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        componentManager->addComponent<T>(entity, component);
        auto signature = entityManager->getSignature(entity);
        signature.set(componentManager->getComponentTypeID<T>(), true);
        entityManager->setSignature(entity, signature);
        systemManager->entitySignatureChanged(entity, signature);
    }

    /**
     * @brief Removes a component from an entity
     * @tparam T Component type to remove
     * @param entity Entity to remove the component from
     */
    template <typename T>
    void removeComponent(Entity entity) {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        componentManager->removeComponent<T>(entity);
        auto signature = entityManager->getSignature(entity);
        signature.set(componentManager->getComponentTypeID<T>(), false);
        entityManager->setSignature(entity, signature);
        systemManager->entitySignatureChanged(entity, signature);
    }

    /**
     * @brief Gets a reference to a component
     * @tparam T Component type to retrieve
     * @param entity Entity owning the component
     * @param force If true, bypasses mutex lock for performance (use with caution)
     * @return Reference to the component
     */
    template <typename T>
    T &getComponent(Entity entity, bool force = false) {
        if (force) {
            // Skip mutex lock if force=true (use with caution - only when thread safety is handled externally)
            return componentManager->getComponent<T>(entity);
        } else {
            std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
            return componentManager->getComponent<T>(entity);
        }
    }

    /**
     * @brief Safely tries to get a component
     * @tparam T Component type to retrieve
     * @param entity Entity to get component from
     * @return Pointer to component, or nullptr if it doesn't exist
     */
    template <typename T>
    T *tryGetComponent(Entity entity) {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        if (hasComponent<T>(entity, true)) {
            return &getComponent<T>(entity, true);
        }
        return nullptr;
    }

    /**
     * @brief Gets the type ID for a component
     * @tparam T Component type
     * @return Component type ID
     */
    template <typename T>
    ComponentTypeID getComponentTypeID() {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        return componentManager->getComponentTypeID<T>();
    }

    /**
     * @brief Checks if an entity has a specific component
     * @tparam T Component type to check for
     * @param entity Entity to check
     * @param force If true, bypasses mutex lock for performance (use with caution)
     * @return True if the entity has the component, false otherwise
     */
    template <typename T>
    bool hasComponent(Entity entity, bool force = false) {
        if (force) {
            // Skip mutex lock if force=true (use with caution)
            auto signature = entityManager->getSignature(entity);
            auto compTypeID = componentManager->getComponentTypeID<T>();
            return signature.test(compTypeID);
        } else {
            std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
            auto signature = entityManager->getSignature(entity);
            auto compTypeID = componentManager->getComponentTypeID<T>();
            return signature.test(compTypeID);
        }
    }

    /**
     * @brief Checks if a pair of entities has a specific pair of components
     * @tparam T1 First component type
     * @tparam T2 Second component type
     * @param entityA First entity
     * @param entityB Second entity
     * @return True if the entities have the components (in either order), false otherwise
     */
    template <typename T1, typename T2>
    bool hasComponentPair(Entity entityA, Entity entityB) {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        return (hasComponent<T1>(entityA, true) && hasComponent<T2>(entityB, true)) ||
               (hasComponent<T2>(entityA, true) && hasComponent<T1>(entityB, true));
    }

    /**
     * @brief Gets the component signature for an entity
     * @param entity Entity to get signature for
     * @return Component signature for the entity
     */
    Signature getEntitySignature(Entity entity) {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        return entityManager->getSignature(entity);
    }

    /**
     * @brief Gets all entities currently in the system
     * @return Vector of all entity IDs
     */
    std::vector<Entity> getEntities() {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        return entityManager->getEntities();
    }

    /**
     * @brief Retrieves all entities that have both specified component types
     * @tparam T1 First component type to check for
     * @tparam T2 Second component type to check for
     * @return Vector containing all entities with both component types
     */
    template <typename T1, typename T2>
    std::vector<Entity> getAllEntitiesWith() {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        std::vector<Entity> entitiesWithComponents;
        for (auto entity : entityManager->getEntities()) {
            if (hasComponent<T1>(entity, true) && hasComponent<T2>(entity, true)) {
                entitiesWithComponents.push_back(entity);
            }
        }
        return entitiesWithComponents;
    }

    /**
     * @brief Sets the component signature for a system
     * @tparam T The system type
     * @param signature The component signature to assign to the system
     *
     * This determines which entities the system will process based on their components.
     */
    template <typename T>
    void setSystemSignature(Signature signature) {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        systemManager->setSignature<T>(signature);
    }

    /**
     * @brief Gets the number of commands waiting in the queue
     * @return Current command count (currently always returns 0)
     * @note Command queue functionality is not yet implemented
     */
    int getEnqueuedCommandsCount() {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        return 0;
    }

    /**
     * @brief Returns a reference to the ECS mutex
     * @return Reference to the shared mutex used for thread synchronization
     * @warning Direct access to the mutex should be used with caution
     */
    std::shared_mutex &getEcsMutex() { return m_ecsMutex; }

private:
    /// Core manager for handling components
    std::unique_ptr<ComponentManager> componentManager;

    /// Core manager for handling entities
    std::unique_ptr<EntityManager> entityManager;

    /// Core manager for handling systems
    std::unique_ptr<SystemManager> systemManager;

    /// Mutex for ensuring thread-safe operations across the ECS
    std::shared_mutex m_ecsMutex;
};
