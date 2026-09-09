# DHARTI Project Change History Index

This directory maintains the chronological history of all structural and functional changes made across iterations of the **DHARTI** platform.

Every iteration that introduces, modifies, or removes functions, models, or architectural components appends an entry here.

---

## Release & Iteration Catalog

| Version | Date | Title / Milestone | Key Highlights |
| :--- | :--- | :--- | :--- |
| **[v0.14.0](v0.14.0_federated_multi_source_scrapers.md)** | 2026-09-09 | Federated Multi-Source Scrapers (15 Portals), Concurrent Orchestrator & UI Scraper Hub | Scaled to 15 authoritative Indian government portals across 6 categories (Clearances, Central LA, State RoRs, Judiciary, GIS/Bhuvan, Treasury/PFMS); pure C++17 `std::async` parallel orchestrator (~566ms); in-memory RFC 6234 SHA-256 deduplication cache; CLI commands (`--list-portals`, `--scrape-all`, `--scrape-portal`); interactive Web UI Federated Scraper Hub with category filter chips and telemetry modals. |
| **[v0.13.0](v0.13.0_geotagged_proof_generalized_search_and_nicci_ai_assistant.md)** | 2026-09-09 | Geotagged Document Proofs, Generalized Search, Optimized Scraper & NICCI AI Assistant | Added sub-meter DGPS geotagged official document proofs with authentic GoI letterheads and mini-maps; generalized multi-criteria search without proposal numbers; parallelized C++ scraper with SHA-256 deduplication cache; multi-corridor revision history; C++ Explanatory Query Engine; and official NICCI AI assistant with Voice Narration & Speech Recognition matching official NIC UI. |
| **[v0.12.1](v0.12.1_ui_redesign_gigw_compliance_and_emblem_fix.md)** | 2026-09-08 | Indian Government Portal UI Visual Alignment & Ashoka Emblem Fix | Fixed SVG unconstrained dimensions bug on Ashoka emblem, added inline containment and CSS constraints, synchronized all portal component CSS rules, added cache buster `v=2.2.0`, and verified visual presentation across all tabs via browser subagent. |
| **[v0.12.0](v0.12.0_real_world_drive_evidence_and_government_portal_ui.md)** | 2026-09-08 | Real-World National Project Drive Evidence & Authentic Indian Government Portal UI | Ingested authentic national expressways into partitioned Google Drive (`dharti/raw/...`); revamped UI into an authentic Indian Government portal (Ashoka emblem, tricolor bar, bilingual branding, GIGW 3.0 footer); interactive project search & live tracking with direct Google Drive links and 7-rule circuit breaker visualizer. |
| **[v0.11.0](v0.11.0_realtime_cpp_scrapers_and_polling_daemon.md)** | 2026-09-08 | Real-Time Pure C++ Web Scrapers, Recurring Polling Daemon & Structured 'dharti' Drive Vault | Pure C++ scrapers for PARIVESH, Bhoomi Rashi, and UP Bhulekh; 1-hour recurring polling daemon; strict `dharti/` partitioned Drive vault; 7-Rule Evidence Gate contradiction quarantine. |
| **[v0.10.0](v0.10.0_pure_cpp_hybrid_storage_and_parivesh_pipeline.md)** | 2026-09-08 | Pure C++ Hybrid Storage & PARIVESH Ingestion Pipeline | Pure C++ SHA-256 cryptographic hasher, PARIVESH Clearance Adapter, 7-Rule Evidence Gate, Google Drive Vault client, and Neon DB 4-layer relational ledger. |
| **[v0.9.0](v0.9.0_google_drive_hybrid_storage.md)** | 2026-09-07 | Google Drive Decoupled Storage & Neon DB Indexing | Integrated Google Drive v3 API for large files, added `evidence_documents` table in Neon DB, and built hybrid upload pipeline. |
| **[v0.8.0](v0.8.0_neon_postgresql_integration.md)** | 2026-09-07 | Neon Serverless PostgreSQL Database Integration | Configured `.env`, `.env.example`, connection verification script, and bootstrapped 6 relational tables with authentic data in Neon cloud DB. |
| **[v0.7.0](v0.7.0_scope_table_and_light_theme_ui.md)** | 2026-09-05 | Scope of Study, Component-Wise Tech & Light Theme UI | Formalized Section 1.4 Scope Table and Section 3.3 Technology Table; delivered modern Light Theme Web UI with Leaflet GIS and interactive manual testing sandbox. |
| **[v0.6.0](v0.6.0_real_data_and_generalized_cli.md)** | 2026-09-05 | Real Data Ingestion, Generalized CLI & Empirical Benchmarks | Zero-dependency C++17 JSON parser, authentic Indian land data assets (`data/`), standalone `bin/dharti_cli.exe`, live wall-clock hardware benchmarks. |
| **[v0.5.0](v0.5.0_modular_architecture_and_adapters.md)** | 2026-09-05 | Modular Documentation, Federated Adapters & Event Store | Modular SRS folder (`docs/srs/`), modular changelogs (`changelogs/`), Land Record, Court & Finance adapters, Bitemporal Event Store. |
| **[v0.4.0](v0.4.0_pure_cpp_refactoring.md)** | 2026-09-05 | 100% Pure C++17 Modular Refactor | Complete removal of all redundant Python code and migration to native C++ headers and implementations. |
| **[v0.3.0](v0.3.0_cpp_domain_core.md)** | 2026-09-05 | C++ Domain Core & Unified Native C API | Implemented Contradiction Engine and SIA Inclusion Engine with C ABI. |
| **[v0.2.0](v0.2.0_native_cpp_acceleration.md)** | 2026-09-05 | Native C++ PCI & Unlock Simulation | High-performance continuous frontage interval merging and bottleneck ranking. |
| **[v0.1.0](v0.1.0_baseline_initialization.md)** | 2026-09-05 | Project Baseline & Specification Synthesis | Repository initialization, baseline architecture, and SIH 26016 synthesis. |

---

## Changelog Policy
1. **Never Delete History**: Every iteration is preserved in its own release document.
2. **Standard Format**: Each changelog entry details `Added`, `Changed`, `Deprecated`, `Removed`, and `Fixed`.
3. **Traceability**: All updates reference the corresponding commit, tests, and documentation chapters.
