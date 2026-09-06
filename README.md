# DHARTI

**Evidence-Backed National Land Acquisition Control Plane**  
*SIH 26016 - High-Performance Pure C++ Modular Architecture*

DHARTI now includes **NyayaBot**, a role-aware evidence copilot that explains parcel blockers, PCI, payment, family-inclusion, policy and audit results using cited DHARTI evidence. It may create review drafts, but it never makes an ownership/entitlement decision or mutates an official parcel, payment or court state.

[![GitHub repo](https://img.shields.io/badge/GitHub-BhaveshBhardwaj%2FDHARTI-blue.svg)](https://github.com/BhaveshBhardwaj/DHARTI)
[![Language](https://img.shields.io/badge/Language-C%2B%2B17-orange.svg)](include/dharti/)
[![Documentation](https://img.shields.io/badge/Docs-SRS%20v1.2.0-green.svg)](docs/SRS.md)
[![Build & Tests](https://img.shields.io/badge/Tests-100%25%20Passed-brightgreen.svg)](tests/cpp/)

---

## 📁 Project Structure

The project is architected in modern C++17 with clean separation of public headers, private implementations, and test suites:

```
dharti/
├── Makefile                                                      # Standard build automation
├── build.ps1                                                     # PowerShell one-click build and test runner
├── CHANGELOG.md                                                  # Complete history of changes
├── README.md                                                     # Project documentation
├── .gitignore                                                    # Clean ignore rules
├── docs/                                                         # Living documentation & specifications
│   ├── SRS.md                                                    # Living Software Requirements Specification
│   ├── ARCHITECTURE.md                                           # Architectural design and principles
│   ├── NYAYABOT_INTEGRATION.md                                   # NyayaBot tool, privacy and production contract
│   └── resources/                                                # Source PDFs and workflow posters
├── include/                                                      # C++ Public Header Directory
│   └── dharti/
│       ├── config/
│       │   └── settings.hpp                                      # Thread-safe environment configuration
│       ├── core/
│       │   ├── base.hpp                                          # BaseService & Health check protocols
│       │   └── event_envelope.hpp                                # Canonical Event Envelope
│       ├── models/
│       │   ├── claimant.hpp                                      # Claimant & Household models
│       │   ├── contradiction.hpp                                 # Exception case types & severities
│       │   ├── parcel.hpp                                        # Parcel state machine & geometries
│       │   └── payment.hpp                                       # Payment records & states
│       ├── services/
│       │   ├── contradiction_engine.hpp                          # Contradiction Detection Engine
│       │   ├── pci_engine.hpp                                    # PCI & Unlock Simulation Engine
│       │   └── sia_inclusion_engine.hpp                          # No-Family-Invisible SIA Engine
│       └── utils/
│           └── logger.hpp                                        # Structured high-throughput C++ Logger
├── src/                                                          # C++ Implementation Directory
│   ├── config/
│   │   └── settings.cpp                                          # Configuration implementation
│   ├── services/
│   │   └── pci_engine.cpp                                        # High-level PCI engine implementation
│   ├── utils/
│   │   └── logger.cpp                                            # Logger implementation
│   └── native/                                                   # Core algorithms and C ABI bindings
│       ├── contradiction_engine.cpp                              # Contradiction engine logic
│       ├── dharti_c_api.cpp                                      # Exported C API implementation
│       ├── dharti_c_api.h                                        # Exported C ABI header
│       ├── pci_engine.cpp                                        # Interval merging and unlock simulation
│       ├── pci_engine.h                                          # Low-level native interval header
│       └── sia_inclusion_engine.cpp                              # SIA inclusion rule logic
└── tests/                                                        # C++ Automated Test Suites
    ├── cpp/
    │   ├── test_dharti_core.cpp                                  # Domain-specific verification
    │   └── test_main.cpp                                         # Comprehensive test runner executable
    └── js/
        └── test_nyaya_bot.js                                     # NyayaBot grounding, privacy and safety tests
```

---

## ⚖️ NyayaBot Evidence Copilot

Open the dashboard and select **Ask NyayaBot** in the bottom-right corner. Example questions:

- `Why is parcel P-118 blocked?`
- `What is the current PCI and highest-unlock parcel?`
- `Are any affected families missing from award or R&R records?`
- `Trace payment status for the selected parcel.`

Every response contains an Answer Receipt, evidence references, source freshness, a confidence label and the human-decision boundary. See [NyayaBot × DHARTI Integration](docs/NYAYABOT_INTEGRATION.md) for the complete role/tool contract and production path.

---

## 🚀 Building and Running Tests

### Option 1: Using PowerShell
```powershell
.\build.ps1
```

### Option 2: Using Make
```bash
make
make test
make test-nyayabot
```

### Option 3: Manual g++ Command
```bash
g++ -O3 -std=c++17 -Wall -Wextra -static -static-libgcc -static-libstdc++ -Iinclude -Isrc/native -o tests/cpp/test_dharti_core.exe tests/cpp/test_main.cpp src/config/settings.cpp src/utils/logger.cpp src/services/pci_engine.cpp src/native/pci_engine.cpp src/native/contradiction_engine.cpp src/native/sia_inclusion_engine.cpp src/native/dharti_c_api.cpp
.\tests\cpp\test_dharti_core.exe
```

---

## 📖 Living Documentation

- **[Software Requirements Specification (SRS)](docs/SRS.md)**: Main living requirements specification for SIH 26016.
- **[Architectural Overview](docs/ARCHITECTURE.md)**: System design patterns and high-performance C++ guidelines.
- **[Changelog](CHANGELOG.md)**: Full chronological history of all features, enhancements, and conversions.
