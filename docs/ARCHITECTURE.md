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

## 5. Suggested Components-Wise Technology Stack

| Layer | System Component | Recommended Technology | Technical Justification |
| :--- | :--- | :--- | :--- |
| **Presentation / GIS** | Interactive National Cockpit & GIS Map | **HTML5, Vanilla CSS3, Vanilla ES6+ JS, Leaflet.js** | Zero-framework-overhead responsive light theme UI with high-contrast accessibility, WKT parcel rendering, and interactive manual testing sandboxes. |
| **Domain Control Plane** | Invariants & State Machines | **Modern C++17 Native Engine (`dharti_core.dll`)** | Microsecond-latency execution, zero memory leaks, decoupled state progression for Parcel, Claimant, Payment, and R&R. |
| **Decision Intelligence** | Corridor PCI & Bottleneck Simulation | **C++17 Introsort & Linear Interval Merging ($O(M \log M)$)** | Sub-millisecond continuous frontage computation ($\approx 0.024 \text{ ms}$ for 1,000 intervals); instantaneous bottleneck ranking. |
| **Serialization** | Zero-Dependency Data Interchange | **C++17 Recursive-Descent Parser (`json.hpp`)** | Single-header, zero-external-dependency JSON parser and serializer with typed accessors and robust error reporting. |
| **Federated Adapters** | External Data Ingestion | **Modular C++17 Adapters (RoR, Cadastral GIS, eCourts CIS, PFMS)** | Enforces strict schema validation, deterministic SHA-256 provenance hashes, and quarantine dead-letter handling. |
| **Immutable Ledger** | Bitemporal Canonical Event Store | **Append-Only Log with SHA-256 Hash Chain** | Preserves dual-axis temporal coordinates ($T_v$ source time vs $T_t$ recorded time) guaranteeing Invariant 1 and 7 auditability. |
| **Document Storage** | Evidence & Proof Packages | **Content-Addressable Storage (CAS) with SHA-256 Checksums** | Version-controlled, tamper-evident repository for Gazette notifications, speaking orders, and Panchnama field photos. |

