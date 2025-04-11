#pragma once

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
    void init() {
        componentManager = std::make_unique<ComponentManager>();
        entityManager = std::make_unique<EntityManager>();
        systemManager = std::make_unique<SystemManager>();
    }

    Entity createEntity() {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        return entityManager->createEntity();
    }

    bool entityExists(Entity entity) {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        return entityManager->entityExists(entity);
    }

    void destroyEntity(Entity entity) {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        entityManager->destroyEntity(entity);
        componentManager->entityDestroyed(entity);
        systemManager->entityDestroyed(entity);
    }

    void destroyEntitySync(Entity entity) {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        entityManager->destroyEntity(entity);
        componentManager->entityDestroyed(entity);
        systemManager->entityDestroyed(entity);
    }

    template <typename T>
    void registerComponent() {
        componentManager->registerComponent<T>();
    }

    template <typename T>
    std::shared_ptr<T> registerSystem() {
        return systemManager->registerSystem<T>();
    }

    template <typename T>
    std::shared_ptr<T> getSystem() {
        return systemManager->getSystem<T>();
    }

    template <typename T>
    void addComponent(Entity entity, T component) {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        componentManager->addComponent<T>(entity, component);
        auto signature = entityManager->getSignature(entity);
        signature.set(componentManager->getComponentTypeID<T>(), true);
        entityManager->setSignature(entity, signature);
        systemManager->entitySignatureChanged(entity, signature);
    }

    template <typename T>
    void removeComponent(Entity entity) {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        componentManager->removeComponent<T>(entity);
        auto signature = entityManager->getSignature(entity);
        signature.set(componentManager->getComponentTypeID<T>(), false);
        entityManager->setSignature(entity, signature);
        systemManager->entitySignatureChanged(entity, signature);
    }

    template <typename T>
    T &getComponent(Entity entity, bool force = false) {
        if (force) {
            return componentManager->getComponent<T>(entity);
        } else {
            std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
            return componentManager->getComponent<T>(entity);
        }
    }

    template <typename T>
    T *tryGetComponent(Entity entity) {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        if (hasComponent<T>(entity, true)) {
            return &getComponent<T>(entity, true);
        }
        return nullptr;
    }

    template <typename T>
    ComponentTypeID getComponentTypeID() {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        return componentManager->getComponentTypeID<T>();
    }

    template <typename T>
    bool hasComponent(Entity entity, bool force = false) {
        if (force) {
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

    template <typename T1, typename T2>
    bool hasComponentPair(Entity entityA, Entity entityB) {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        return (hasComponent<T1>(entityA, true) && hasComponent<T2>(entityB, true)) ||
               (hasComponent<T2>(entityA, true) && hasComponent<T1>(entityB, true));
    }

    Signature getEntitySignature(Entity entity) {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        return entityManager->getSignature(entity);
    }

    std::vector<Entity> getEntities() {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        return entityManager->getEntities();
    }

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

    template <typename T>
    void setSystemSignature(Signature signature) {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        systemManager->setSignature<T>(signature);
    }

    int getEnqueuedCommandsCount() {
        std::lock_guard<std::shared_mutex> lock(m_ecsMutex);
        return 0;
    }

    std::shared_mutex &getEcsMutex() { return m_ecsMutex; }

private:
    std::unique_ptr<ComponentManager> componentManager;
    std::unique_ptr<EntityManager> entityManager;
    std::unique_ptr<SystemManager> systemManager;

    std::shared_mutex m_ecsMutex;
};
