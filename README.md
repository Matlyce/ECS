# ECS Engine

## Introduction

An Entity-Component-System (ECS) is a software architectural pattern commonly used in game development and other performance-critical applications. It provides a way to manage entities and their associated data (components) in a highly modular and efficient manner. ECS decouples data from behavior, allowing for better scalability, maintainability, and performance.

### What is an ECS?
- **Entity**: Represents a unique identifier for an object in the system. Entities themselves do not contain data or behavior.
- **Component**: Represents a piece of data associated with an entity. Components are plain data structures without logic.
- **System**: Encapsulates behavior and operates on entities that match specific criteria (signatures). Systems process components to implement game logic or other functionality.

### Why Use an ECS?
- **Performance**: ECS leverages cache-friendly data layouts and minimizes memory fragmentation. Operations like filtering entities by components are highly optimized.
- **Modularity**: Components and systems are independent, making it easy to add, remove, or modify functionality without affecting other parts of the codebase.
- **Scalability**: ECS can handle thousands of entities and components efficiently, making it suitable for large-scale simulations or games.
- **Flexibility**: The decoupled nature of ECS allows for dynamic composition of entities and easy experimentation with different behaviors.

### Use Cases for ECS
- **Game Development**: ECS is widely used in game engines to manage game objects, physics, AI, rendering, and more.
- **Simulations**: ECS is ideal for simulations involving large numbers of entities, such as particle systems, crowd simulations, or traffic models.
- **Data-Driven Applications**: ECS can be used in applications where behavior is determined by data rather than hardcoded logic.
- **Prototyping**: ECS allows for rapid prototyping of new features or mechanics by composing entities dynamically.

## Features
- **Entity Management**: Efficient creation, destruction, and recycling of entities.
- **Component System**: Generic and extensible component storage and management.
- **System Management**: Signature-based entity filtering and system updates.
- **Coordinator**: Unified interface for managing entities, components, and systems.
- **Optimizations**: Cache-friendly structures, bitwise operations, and O(1) complexity for most operations.

## Technical Documentation
For a detailed technical analysis of the ECS engine, including its architecture, algorithmic complexity, and design choices, refer to the [Technical Documentation](ECS.md).

## File Structure
```
ECS/
├── includes/
│   ├── AComponentStorage.hpp
│   ├── ComponentManager.hpp
│   ├── ComponentStorage.hpp
│   ├── Coordinator.hpp
│   ├── ECS.hpp
│   ├── EntityManager.hpp
│   ├── System.hpp
│   ├── SystemManager.hpp
│   └── Types.hpp
├── CMakeLists.txt
├── ECS.md
├── LICENSE
└── README.md
```

## Getting Started

### Prerequisites
- A C++ compiler supporting C++17 or later.
- CMake for build configuration.

### Integration

This ECS engine is designed to be used as a header-only library in your C++ projects. Here's how to integrate it:

#### Option 1: Using CMake (recommended)
1. Clone the repository into your project:
    ```bash
    git clone <repository-url> external/ECS
    ```

2. Add the following to your project's `CMakeLists.txt`:
    ```cmake
    # Add ECS as a subdirectory
    add_subdirectory(external/ECS)

    # Link your target with the ECS library
    target_link_libraries(your_application PRIVATE ECS)
    ```

3. Include the main header in your code:
    ```cpp
    #include "ECS/ECS.hpp"
    ```

#### Option 2: Manual Integration
1. Clone the repository:
    ```bash
    git clone <repository-url>
    ```

2. Copy the `includes` directory to your project

3. Include the main header in your code:
    ```cpp
    #include "ECS.hpp"
    ```

### Usage Example

> Please note that this is a **simplified example**. In a real-world application, the ECS engine should be encapsulated in a more structured way, and you would typically have separate files for components, systems, and the main application logic. The main loop in this example is also simplified for clarity, in a real application, you should take care of event handling, frame rating, and other game loop concerns.
<br>

But anyway, here is a simple example of how to use the ECS engine:
<br>

```cpp
#include "ECS.hpp"

// Global coordinator definition
Coordinator gCoordinator;

// Component definitions
struct PositionComponent {
    float x, y, z;
};

struct VelocityComponent {
    float dx, dy, dz;
};

struct RenderComponent {
    float scale;
    int color;
};

// System definitions
class PhysicsSystem : public System {
public:
    void update(float dt) {
        for (auto entity : entities) {
            auto& position = gCoordinator.getComponent<PositionComponent>(entity);
            const auto& velocity = gCoordinator.getComponent<VelocityComponent>(entity);

            position.x += velocity.dx * dt;
            position.y += velocity.dy * dt;
            position.z += velocity.dz * dt;
        }
    }
};

class RenderSystem : public System {
public:
    void render() {
        for (auto entity : entities) {
            const auto& position = gCoordinator.getComponent<PositionComponent>(entity);
            const auto& render = gCoordinator.getComponent<RenderComponent>(entity);

            // Here you would integrate your rendering code
            // DrawCube(position.x, position.y, position.z, render.scale, render.color);
        }
    }
};

int main() {
    // Initialize coordinator
    gCoordinator.init();

    // Register components
    gCoordinator.registerComponent<PositionComponent>();
    gCoordinator.registerComponent<VelocityComponent>();
    gCoordinator.registerComponent<RenderComponent>();

    // Create and configure systems
    auto physicsSystem = gCoordinator.registerSystem<PhysicsSystem>();
    Signature physicsSignature;
    physicsSignature.set(gCoordinator.getComponentTypeID<PositionComponent>(), true);
    physicsSignature.set(gCoordinator.getComponentTypeID<VelocityComponent>(), true);
    gCoordinator.setSystemSignature<PhysicsSystem>(physicsSignature);

    auto renderSystem = gCoordinator.registerSystem<RenderSystem>();
    Signature renderSignature;
    renderSignature.set(gCoordinator.getComponentTypeID<PositionComponent>(), true);
    renderSignature.set(gCoordinator.getComponentTypeID<RenderComponent>(), true);
    gCoordinator.setSystemSignature<RenderSystem>(renderSignature);

    // Create entities
    auto cube = gCoordinator.createEntity();
    gCoordinator.addComponent<PositionComponent>(cube, {0.0f, 0.0f, 0.0f});
    gCoordinator.addComponent<VelocityComponent>(cube, {1.0f, 0.5f, 0.0f});
    gCoordinator.addComponent<RenderComponent>(cube, {1.0f, 0xFFFF0000}); // Red cube

    auto sphere = gCoordinator.createEntity();
    gCoordinator.addComponent<PositionComponent>(sphere, {5.0f, 0.0f, 0.0f});
    gCoordinator.addComponent<VelocityComponent>(sphere, {-0.5f, 0.0f, 1.0f});
    gCoordinator.addComponent<RenderComponent>(sphere, {0.5f, 0xFF0000FF}); // Blue sphere

    // Simple game loop
    const float fixedTimeStep = 0.016f; // ~60fps
    float accumulator = 0.0f;

    // Use high resolution clock for precise timing
    std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();

    // Game loop
    while (true) { // In a real application, you would have an exit condition
        // Calculate delta time
        auto newTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(newTime - currentTime).count();
        currentTime = newTime;

        // Update with fixed timestep for physics stability
        accumulator += dt;
        while (accumulator >= fixedTimeStep) {
            physicsSystem->update(fixedTimeStep);
            accumulator -= fixedTimeStep;
        }

        // Render
        renderSystem->render();

        // Here you would wait for the next frame or handle events
    }
    return 0;
}
```

The examples above provide a basic overview of how to use the ECS engine. However, the library offers many more features and possibilities that you might want to explore.

### Consulting the Source Code

For a complete understanding of the API and available methods, it's recommended to consult the source code directly. The codebase includes comprehensive Doxygen documentation comments that explain:

- Class responsibilities and relationships
- Method parameters and return values
- Usage patterns and constraints
- Performance considerations

The self-documenting nature of the code makes it an excellent reference for understanding all available functionality and implementation details.

### API Documentation

A complete API documentation generated with Doxygen is available online at [https://matlyce.github.io/ECS/](https://matlyce.github.io/ECS/). This documentation is automatically updated when changes are pushed to the main branch.

### Key Files to Explore

- **Coordinator.hpp**: The main interface for interacting with the ECS system
- **EntityManager.hpp**: Entity creation, destruction and management
- **ComponentManager.hpp**: Component registration and association
- **SystemManager.hpp**: System registration and signature management
- **Types.hpp**: Core type definitions and constants

## License & Contributing
This project is licensed under the terms of the [LICENSE](LICENSE) file.

Contributions to this project are welcome! If you have ideas for improvements, optimizations, or new features, feel free to fork the repository and submit a pull request.
<br>
Please ensure your contributions align with the project's goals and maintain the code quality.

## Credit

Made with ❤️ by Matlyce
<br>
*If you find this project useful, please consider giving it a star on GitHub!* ⭐

## Note

> This project was made during my third year at EPITECH school (in France). If you are a EPITECH student working on a similar project, do not copy this code into your project as it will be flagged by EPITECH's ANTI-CHEAT system, resulting in a -42 grade. However, you are welcome to take inspiration from the concepts of this ECS implementation.
