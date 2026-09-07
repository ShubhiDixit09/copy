# AGENTS.md - DHARTI Project Instructions

## Non-Negotiable Directives
1. **Primary Language**: C++17/C++20. Minimize Python. Write collectors, engines, adapters, storage clients, and models in modular C++.
2. **Changelog**: Must be a folder (`changelogs/`). Create an incremental changelog file for each major function addition/removal and update `changelogs/index.md`.
3. **SRS**: Must be a folder (`docs/srs/`). Update SRS files as features evolve.
4. **Feasibility Analysis**: Store side-by-side analysis in `docs/analysis/`.
5. **Real-World & Generalizability**: Calculations (PCI, contradiction detection, SHA-256) and tests must use real data structures and real logic.
6. **Architecture**:
   - **Neon PostgreSQL (with PostGIS)**: Control plane database for metadata, indexes, source health, bitemporal workflow events, PostGIS geometry. No large binary blobs in Neon.
   - **Google Drive**: Immutable evidence vault partitioned by source/date. Stores PDFs, raw snapshots.
   - **Pipeline**: `AUTHORITATIVE SOURCES -> C++ COLLECTOR -> DRIVE VAULT + NEON DB -> CHANGE DETECTOR -> CANONICAL EVENT -> 7-RULE EVIDENCE GATE (ACCEPT/QUARANTINE) -> PROJECTIONS & PCI -> DASHBOARD`.
