/**
 * @file System.hpp
 * @brief Defines the base System class for the ECS framework
 */
#pragma once

#include "Types.hpp"
#include <atomic>
#include <cmath>
#include <functional>
#include <iostream>
#include <set>

/**
 * @class System
 * @brief Base class for all systems in the ECS architecture
 *
 * Systems operate on entities that have specific component signatures.
 * Each system implements logic to process entities with matching components.
 */
class System {
public:
    /** Set of entities that match this system's required component signature */
    std::set<Entity> entities;

    /**
     * @brief Get all entities currently managed by this system
     * @return Set of entity IDs that this system operates on
     */
    std::set<Entity> getEntities() const {
        return entities;
    }

    /**
     * @brief Update method to be overridden by derived systems
     * @param deltaTime Time elapsed since last update (in seconds)
     */
    virtual void update(float deltaTime) {}

    /**
     * @brief Render method to be overridden by derived systems
     */
    virtual void render() {}

    /**
     * @brief Accumulate delta time for time-based execution
     * @param deltaTime Time to add to the accumulated delta
     */
    void addDelta(float deltaTime) { _delta += deltaTime; }

    /**
     * @brief Get current accumulated delta time
     * @return Current accumulated delta time in seconds
     */
    float getDelta() const { return _delta; }

    /**
     * @brief Check if system has accumulated enough time to execute
     * @return True if system can execute based on its threshold
     */
    bool canExecute() const {
        return _delta >= _threshold;
    }

    /**
     * @brief Set the system's execution frequency in ticks per second (TPS)
     * @param tps Desired execution frequency
     *
     * This converts the TPS value to a time threshold between executions
     */
    void setTPS(float tps) {
        _threshold = 1.0f / tps;
    }

    /**
     * @brief Get the current execution threshold
     * @return Time threshold between executions in seconds
     */
    float getThreshold() const {
        return _threshold;
    }

    /**
     * @brief Execute a task when enough time has accumulated
     * @param deltaTime Current frame's delta time
     * @param task Function to execute when threshold is reached
     * @param scheduler Function that schedules the task execution (e.g. thread pool)
     *
     * This method provides thread-safe execution of system logic based on time thresholds.
     * It prevents multiple concurrent executions of the same system by using atomic flags.
     */
    void executeWhenPossible(float deltaTime,
                             std::function<void(float)> task,
                             std::function<void(std::function<void()>)> scheduler) {
        addDelta(deltaTime);
        if (canExecute() && !_isTaskRunning.load(std::memory_order_acquire)) {
            _isTaskRunning.store(true, std::memory_order_release);
            float totalDelta = getThreshold();

            scheduler([this, totalDelta, task]() {
                task(totalDelta);
                _isTaskRunning.store(false, std::memory_order_release);
            });

            _delta = 0.0f;
        }
    }

private:
    /** Accumulated delta time since last execution */
    float _delta = 0.0f;

    /** Time threshold between executions (default: 1/30 seconds) */
    float _threshold = 1.0f / 30.0f;

    /** Flag to prevent concurrent execution of the system */
    std::atomic_bool _isTaskRunning{false};
};
