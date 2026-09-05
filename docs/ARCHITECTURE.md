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

## 2. Directory Structure & Responsibilities (Pure C++17)

- **`include/dharti/core/`**: Defines baseline C++ interfaces (`base.hpp`), health protocols, and the canonical `event_envelope.hpp`.
- **`include/dharti/models/`**: Strongly typed domain models (`parcel.hpp`, `claimant.hpp`, `payment.hpp`, `contradiction.hpp`) and decoupled state machines.
- **`include/dharti/services/`**: C++ domain service headers (`contradiction_engine.hpp`, `sia_inclusion_engine.hpp`, `pci_engine.hpp`).
- **`include/dharti/config/`**: Thread-safe configuration manager (`settings.hpp`).
- **`include/dharti/utils/`**: Structured logger and utilities (`logger.hpp`).
- **`src/`**: Private C++ implementation source files (`config/settings.cpp`, `utils/logger.cpp`, `services/pci_engine.cpp`).
- **`src/native/`**: Core mathematical algorithms (`pci_engine.cpp`, `contradiction_engine.cpp`, `sia_inclusion_engine.cpp`) and exported C ABI (`dharti_c_api.cpp`).
- **`tests/cpp/`**: Comprehensive C++ test suite (`test_main.cpp`) verifying 100% of domain rules.
- **`docs/`**: Project documentation, specifications, living `SRS.md`, and PDF resources.

## 3. Pure C++ Performance & Architecture Strategy
1. **Zero Runtime Overhead**:
   - Zero-overhead data structures (arrays, contiguous vectors, minimal heap allocation).
   - Microsecond latency for interval intersections, continuous corridor merging, and contradiction evaluations.
2. **Deterministic Execution**:
   - Explicit state machines for Parcel, Claimant, Payment, and R&R.
   - Deterministic contradiction detection with zero silent overwrites.
3. **Build Automation & Portability**:
   - Standard `Makefile` and `build.ps1` for building `dharti_core.dll` and running `test_dharti_core.exe` with standard `g++`.

## 4. Modularity Guidelines
1. **No Circular Dependencies**: Lower layers must never depend on higher layers.
2. **Explicit Contracts**: Cross-module communication should occur through well-defined function signatures or interface classes.
3. **Incremental Commits**: Each feature module must be committed with its corresponding unit tests and documentation updates.
