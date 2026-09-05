# DHARTI Architectural Overview

## 1. Architectural Style
DHARTI is organized using **Clean Architecture** and **Domain-Driven Design (DDD)** principles to guarantee high modularity:

```
                  +-----------------------------------+
                  |             Interfaces            |
                  |     (API / CLI / Presentation)    |
                  +-----------------+-----------------+
                                    |
                                    v
                  +-----------------+-----------------+
                  |         Application Services      |
                  |       (Use Cases & Workflows)     |
                  +-----------------+-----------------+
                                    |
                                    v
                  +-----------------+-----------------+
                  |           Core Domain             |
                  |     (Entities, Models, Rules)     |
                  +-----------------+-----------------+
                                    |
                                    v
                  +-----------------+-----------------+
                  |          Infrastructure           |
                  |   (Data Access, Utils, External)  |
                  +-----------------------------------+
```

## 2. Directory Structure & Responsibilities

- **`src/core/`**: Defines baseline interfaces, abstractions, settings, and constants that all modules can rely upon.
- **`src/models/`**: Domain entities, schemas, and value objects.
- **`src/services/`**: Business logic implementations isolated from delivery mechanisms (API/CLI).
- **`src/native/`**: High-performance C++ native routines for geometric interval intersections, PCI computation, and large-scale corridor unlock simulations.
- **`src/api/`**: Delivery mechanisms, endpoints, request handlers, and DTO serializers.
- **`src/utils/`**: General reusable utilities, logging, validation helpers.
- **`config/`**: Configuration management and environment settings.
- **`tests/`**: Unit and integration test suites matching the `src/` modular layout.
- **`docs/`**: Project documentation, design specifications, and the living `SRS.md`.

## 3. Native C++ Acceleration Strategy
1. **Target Domains**:
   - Continuous chainage interval sorting and merging.
   - Exhaustive combinatorial unlock simulations across $N > 10,000$ parcels.
   - High-throughput spatial polygon-corridor intersection calculations.
2. **Interoperability**:
   - Clean C-compatible export interface (`extern "C"`).
   - Zero-overhead data structures (arrays/contiguous memory blocks).
   - Python `ctypes` wrapper with pure Python fallback for seamless environments.

## 4. Modularity Guidelines
1. **No Circular Dependencies**: Lower layers must never depend on higher layers.
2. **Explicit Contracts**: Cross-module communication should occur through well-defined function signatures or interface classes.
3. **Incremental Commits**: Each feature module must be committed with its corresponding unit tests and documentation updates.
