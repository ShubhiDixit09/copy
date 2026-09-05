# Changelog

All notable changes to the **DHARTI** project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [Unreleased]

---

## [0.2.0] - 2026-09-05

### Added
- **C++ Native Acceleration Engine** (`src/native/pci_engine.h`, `src/native/pci_engine.cpp`):
  - C ABI interface (`extern "C"`) for sub-millisecond continuous chainage interval sorting and merging.
  - Native calculation of the Possession Continuity Index (PCI = $L_{\text{continuous}}^{\max} / L_{\text{total}}$).
  - Combinatorial $O(N)$ corridor unlock simulation ranking bottleneck parcels by frontage unlock gain.
- **Dual-Mode Python Service** (`src/services/pci/pci_service.py`):
  - Dynamic `ctypes` loading of statically linked C++ shared library (`pci_engine.dll` / `.so`).
  - Seamless fallback to pure Python algorithm if C++ compiler or native binary is absent.
- **PCI Engine Test Suite** (`tests/unit/test_pci_engine.py`):
  - Validates Python and C++ implementations for 100% mathematical and simulation parity.
  - Tests corridor bottleneck identification (e.g. Parcel P-118 yielding +5.0 km continuous frontage unlock).
- **SRS & Architecture Enhancements**:
  - Section 7.1.1 added to `docs/SRS.md` detailing Native C++ Acceleration requirements.
  - Section 3 added to `docs/ARCHITECTURE.md` documenting performance architecture.

### Changed
- Updated `.gitignore` to ignore compiled native binaries (`*.dll`, `*.dylib`) while keeping source files version-controlled.

---

## [0.1.0] - 2026-09-05

### Added
- **Repository Initialization**:
  - Linked workspace to remote repository [`https://github.com/BhaveshBhardwaj/DHARTI.git`](https://github.com/BhaveshBhardwaj/DHARTI.git) on branch `main`.
  - Configured comprehensive `.gitignore` for Python, Node, OS, and environment files.
- **Living Specifications**:
  - Initialized `docs/SRS.md` with complete SIH 26016 problem requirements:
    - 4 Core Jobs: Exchange, Prove, Coordinate, Decide.
    - 7 Non-Negotiable Invariants.
    - 8 Statutory Workflow Stages (Initiate to Assure).
    - 4 Decoupled State Machines (Parcel, Claimant, Payment, R&R).
    - Core Deterministic Algorithms (PCI, Contradiction Engine, No-Family-Invisible SIA Safeguard, Delay Intelligence).
    - Canonical Event Envelope and Integration Adapters specification.
  - Created `docs/ARCHITECTURE.md` detailing Clean Architecture and Domain-Driven Design layout.
  - Organized problem statement and workflow posters into `docs/resources/`.
- **Modular Codebase Scaffolding**:
  - `config/settings.py`: Centralized application configuration.
  - `src/utils/logger.py`: Standard structured logging utility.
  - `src/core/base.py`: Abstract base service interface.
  - Modular packages initialized under `src/core/`, `src/models/`, `src/services/`, `src/api/`, `src/utils/`.
- **Automated Testing**:
  - Baseline test suite initialized in `tests/unit/test_baseline.py`.
