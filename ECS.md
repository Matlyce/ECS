# Detailed Technical Analysis of the ECS Engine:

1. **Core Architecture**
- Based on unique identifiers (Entity) using `uint32_t`
- Templates used for component generalization
- Signatures implemented via `std::bitset<64>` for highly efficient binary mapping
- Memory management through smart pointers (`unique_ptr`, `shared_ptr`)

2. **Entity Management (EntityManager)**
- Entity pool with recycling using `std::queue<Entity>`
- Maximum capacity of 5000 entities configured in `Types.hpp`
- Signatures stored in a static array `std::array<Signature, MAX_ENTITIES>`
- Live entity tracking via `livingEntityCount`
- O(1) methods for creation/destruction

3. **Component System**
- **Storage (ComponentStorage)**
  - Generic template `ComponentStorage<T>`
  - Uses `unordered_map<Entity, T>` for O(1) access
  - Abstract interface via `AComponentStorage`
  - Polymorphism for uniform management

- **ComponentManager Overview**
  - Assigns unique IDs to component types using `ComponentTypeID`
  - Maintains two mappings:
    - `unordered_map<type_index, ComponentTypeID>` for type-to-ID mapping
    - `unordered_map<type_index, shared_ptr<AComponentStorage>>` for type-to-storage mapping
  - Uses `type_index` for identifying component types
  - Supports dynamic registration of new components

4. **System Management (SystemManager)**
- Systems stored via `unordered_map<type_index, shared_ptr<System>>`
- Associated signatures in `unordered_map<type_index, Signature>`
- Entity filtering by signature matching:
  ```cpp
  (entitySignature & systemSignature) == systemSignature
  ```
- Relevant entities cached in `std::set`

5. **Coordinator**
- Facade pattern for the public API
- Deferred destruction management via `entitiesToDestroy`
- Unified interface for all operations
- Component validity checks
- Dependency management between systems

6. **Technical Optimizations**
- Use of references to avoid copies
- Move semantics for component transfers
- Bitwise operations for signature matching
- Cache-friendly with contiguous structures

7. **Advanced Features**
- Support for multi-component queries
- Component pair validation
- Deferred destruction to avoid invalidations
- Extensibility through templates

This implementation is designed for efficiency and flexibility. Most operations are O(1), ensuring high performance. The use of templates and polymorphism allows for adaptability, while the modular design promotes scalability and maintainability.

<br>

# Algorithmic Complexity for Major ECS Operations

### EntityManager.hpp

**O(1) Operations:**
- `createEntity()` : O(1) - Uses `queue.front()` and `queue.pop()`
- `destroyEntity()` : O(1) - Resets the bitset and `queue.push()`
- `setSignature()` : O(1) - Direct array access via index
- `getSignature()` : O(1) - Direct array access via index

**O(n) Operations:**
- `getEntities()` : O(n) - Iterates over `MAX_ENTITIES` to construct the vector

### ComponentManager.hpp

**O(1) Operations:**
- `registerComponent<T>()` : O(1) - Inserts into `unordered_map`
- `addComponent<T>()` : O(1) - Inserts into `ComponentStorage`
- `removeComponent<T>()` : O(1) - Removes from `unordered_map`
- `getComponent<T>()` : O(1) - Direct access via `unordered_map`
- `getComponentTypeID<T>()` : O(1) - Lookup in `unordered_map`

**O(m) Operations:** (where m = number of component types)
- `entityDestroyed()` : O(m) - Iterates over all `ComponentStorages`

### SystemManager.hpp

**O(1) Operations:**
- `registerSystem<T>()` : O(1) - Inserts into `unordered_map`
- `getSystem<T>()` : O(1) - Direct access via `unordered_map`
- `setSignature()` : O(1) - Inserts into `unordered_map`

**O(s) Operations:** (where s = number of systems)
- `entityDestroyed()` : O(s) - Iterates over all systems
- `entitySignatureChanged()` : O(s) - Iterates over all systems

### ComponentStorage.hpp

**All operations are O(1):**
- `insertData()` : O(1) - Inserts into `unordered_map`
- `removeData()` : O(1) - Removes from `unordered_map`
- `getData()` : O(1) - Direct access via `unordered_map`
- `hasData()` : O(1) - Lookup in `unordered_map`

### Coordinator.hpp

**Inherited O(1) Operations:**
- `createEntity()`, `addComponent()`, `removeComponent()`, `getComponent()`

**Composite Operations:**
- `destroyEntity()` : O(1) - Simple `push_back` into vector
- `processEntityDestruction()` : O(n + m + s) where:
  - n = number of entities to destroy
  - m = number of component types
  - s = number of systems

**Validation Operations:**
- `hasComponent()` : O(1) - Bit test in signature
- `hasComponentPair()` : O(1) - Two calls to `hasComponent`

### Performance Critical Points:

1. **Signature Management:**
```cpp
(entitySignature & systemSignature) == systemSignature  // O(1) - bitwise operation
```

2. **Component Storage:**
```cpp
std::unordered_map<Entity, T> componentMap;  // Average O(1) access
```

3. **Cache Optimization:**
```cpp
std::array<Signature, MAX_ENTITIES> signatures{};  // Contiguous memory
```
The overall system complexity is optimized for common operations (O(1) for most entity and component interactions), with some more expensive operations during entity destruction or system updates.

## Conclusion
This implementation might not be the most optimal or exhaustive, it has proven sufficient for the needs of my project so I decided to keep it as is.
<br>
Hope this helps you understand my implementation of the ECS engine and the design choices I made.

