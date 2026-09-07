# DHARTI Core Rules & Architectural Directives

These non-negotiable rules govern the DHARTI (National Land Acquisition & Management System - SIH 26016) codebase:

1. **C++ First Architecture**:
   - Write code in C++17/C++20 wherever possible.
   - Remove `.py` files and replace with C++ implementations unless an external tool or script strictly requires another language.
   - Keep all code modular, object-oriented, clean, and adhering to ISO C++ standard practices.

2. **Changelog Folder Maintenance**:
   - Maintain `changelogs/` as a directory with incremental versioned files (e.g. `changelogs/v0.X.Y_feature_name.md`) and keep `changelogs/index.md` up to date.
   - Update changelog logs after each iteration of new functions or removal of functions.

3. **SRS as a Folder**:
   - Maintain `docs/srs/` as a structured directory with modular requirement specifications.
   - Update SRS after evaluation of new components.

4. **Architecture & Feasibility Analysis**:
   - Strictly implement `docs/resources` and `docs/ARCHITECTURE.md`.
   - Analyze each architectural aspect carefully for feasibility and effectiveness, saving findings side by side in `docs/analysis/`.
   - Comprehensive explainability from top to bottom across all documentation and source files.

5. **Zero Simulation / Real-World Data**:
   - All engines, calculations, and tests must operate on real, empirical, generalized data.
   - No mock approximations or fake hardcoded assertions; calculations (e.g., PCI continuous frontage, polygon areas, SHA-256 hashes, stay orders) must be mathematically and cryptographically real.

6. **Hybrid Cloud Architecture (₹0 Operational Cost)**:
   - **Neon Serverless PostgreSQL + PostGIS**: Control plane database storing metadata, indexes, SHA-256 checksums, source health, bitemporal workflow events, and PostGIS parcel geometry. Raw files/large PDFs are NEVER dumped into Neon.
   - **Google Drive Webhook / API Vault**: Immutable object store partitioned as `DHARTI_EVIDENCE_VAULT/raw/{source}/{YYYY}/{MM}/{DD}/snapshot_XXX.json`. Previous snapshots are NEVER overwritten. Neon stores the `drive_file_id`, `drive_url`, `sha256`, and mime metadata.
   - **Phase 1 Vertical Slice**:
     `PARIVESH -> C++ Collector/Adapter -> Google Drive Evidence Vault -> Neon DB (Snapshots & Artifacts) -> C++ Change Detector -> Canonical Event Envelope -> C++ 7-Rule Evidence Gate (ACCEPT / QUARANTINE) -> State Projections & PCI -> Dashboard/CLI`.
