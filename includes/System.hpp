#pragma once

#include "Types.hpp"
#include <atomic>
#include <cmath>
#include <functional>
#include <iostream>
#include <set>

/**
 * @class System
 * @brief Base class for all systems.
 */
class System {
public:
    std::set<Entity> entities;

    std::set<Entity> getEntities() const {
        return entities;
    }

    virtual void update(float deltaTime) {}
    virtual void render() {}

    void addDelta(float deltaTime) { _delta += deltaTime; }
    float getDelta() const { return _delta; }

    bool canExecute() const {
        return _delta >= _threshold;
    }

    void setTPS(float tps) {
        _threshold = 1.0f / tps;
    }

    float getThreshold() const {
        return _threshold;
    }

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
    float _delta = 0.0f;
    float _threshold = 1.0f / 30.0f;
    std::atomic_bool _isTaskRunning{false};
};
