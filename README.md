# DHARTI

DHARTI is a modular, extensible, and high-performance software system designed for maintainability and scalability.

[![GitHub repo](https://img.shields.io/badge/GitHub-BhaveshBhardwaj%2FDHARTI-blue.svg)](https://github.com/BhaveshBhardwaj/DHARTI)
[![Documentation](https://img.shields.io/badge/Docs-SRS%20v1.0.0-green.svg)](docs/SRS.md)

---

## 📁 Project Structure

The project is architected with strict separation of concerns:

```
dharti/
├── config/               # Application configuration and settings
├── docs/                 # Software Requirements & Architectural documentation
│   ├── SRS.md            # Living Software Requirements Specification
│   └── ARCHITECTURE.md   # Architectural design and guidelines
├── src/                  # Core modular source code
│   ├── api/              # API interfaces, routers, and controllers
│   ├── core/             # Base abstractions, interfaces, and core components
│   ├── models/           # Domain models, data structures, and schemas
│   ├── services/         # Business logic and domain services
│   └── utils/            # Helper utilities and shared libraries
├── tests/                # Automated test suites
│   ├── unit/             # Unit tests
│   └── integration/      # Integration tests
├── .gitignore            # Git ignore configuration
└── README.md             # Project documentation
```

---

## 📖 Documentation

- **[Software Requirements Specification (SRS)](docs/SRS.md)**: Main requirements document, updated iteratively as new features and documents are introduced.
- **[Architectural Overview](docs/ARCHITECTURE.md)**: System design principles, layers, and modularity guidelines.

---

## 🛠️ Development & Workflow

- Each feature or function is implemented in a self-contained module.
- For each new feature:
  1. Append requirements to [docs/SRS.md](docs/SRS.md).
  2. Implement code under `src/`.
  3. Add corresponding tests under `tests/`.
  4. Perform atomic git commit and push to remote.
