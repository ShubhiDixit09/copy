# Chapter 3: Logical Architecture & Components

## 3.1 Tiered Control Plane Architecture

```
+---------------------------------------------------------------------------------------------------+
|                                  1. CHANNELS & DECISION SURFACES                                  |
|   National/State Cockpit   |   District Workbench   |   Field PWA (Offline)  |   Auditor Portal   |
+-------------------------------------------------+-------------------------------------------------+
                                                  |
                                                  v
+---------------------------------------------------------------------------------------------------+
|                                  2. SECURE ACCESS EDGE GATEWAY                                    |
|         ePramaan / National SSO  |  ABAC + RBAC  |  Field-Level Redaction  |  Audit Interceptor    |
+-------------------------------------------------+-------------------------------------------------+
                                                  |
                                                  v
+---------------------------------------------------------------------------------------------------+
|                                      3. DOMAIN CONTROL PLANE                                      |
|    Alignment & Corridor   |    Parcel Lineage & GIS    |    Statutory Clocks & SLA Workflow       |
|    Claimant & Household   |    Award & Obligations     |    Payment Reconciliation Engine         |
+-------------------------------------------------+-------------------------------------------------+
                                                  |
                                                  v
+---------------------------------------------------------------------------------------------------+
|                                  4. TRUST & DECISION INTELLIGENCE                                 |
|  +---------------------------+  +---------------------------+  +-------------------------------+  |
|  | Contradiction Engine      |  | PCI & Unlock Simulator    |  | No-Family-Invisible SIA Rule  |  |
|  | (Deterministic Conflicts) |  | (Continuity Mathematics)  |  | (Exclusion Protection)        |  |
|  +---------------------------+  +---------------------------+  +-------------------------------+  |
+-------------------------------------------------+-------------------------------------------------+
                                                  |
                                                  v
+---------------------------------------------------------------------------------------------------+
|                                  5. CANONICAL EVENT & PERSISTENCE                                 |
|       Bitemporal Event Store  |  PostgreSQL / PostGIS  |  Versioned S3/MinIO Proof Packages       |
+-------------------------------------------------+-------------------------------------------------+
                                                  ^
                                                  |
+-------------------------------------------------+-------------------------------------------------+
|                                    6. FEDERATED SOURCE ADAPTERS                                   |
|   Land Record & Cadastral GIS  |  eCourts / Revenue Courts (RCCMS)  |  PFMS & Bank Gateway        |
+---------------------------------------------------------------------------------------------------+
```

## 3.2 Decision Surfaces & Operational Roles
- **National / State Cockpit**: Provides executive corridor visibility, Possession Continuity Index (PCI), bottleneck forecasts, and cross-state comparisons without raw personal data.
- **District Workbench**: Tailored for the District Collector and Land Acquisition Authority (LAA) to manage hearing dockets, speaking orders, valuation reviews, and exception resolution.
- **Field PWA (Offline-First)**: Equips survey and possession teams with encrypted local queues for geotagged/timestamped photography, witness memoranda, and offline survey synchronization.
- **Auditor & Public View**: Enables complete historical reconstruction for CAG/statutory auditors, while presenting generalized public transparency dashboards.

---

## 3.3 Suggested Components-Wise Technology Table

The following table provides the recommended, production-grade technology selections across all architectural tiers of the National Land Acquisition & Management System:

| Architectural Tier | System Component | Suggested Technology | Role & Technical Justification |
| :--- | :--- | :--- | :--- |
| **Tier 1: Channels & Presentation** | Web-based National Cockpit & Light Theme UI | **HTML5, Modern Vanilla CSS3, Vanilla ES6+ JavaScript** | High-performance, zero-framework-overhead responsive web interface. Provides a pristine, high-contrast light theme design system with accessible typography, glassmorphism cards, and interactive manual testing sandboxes. |
| **Tier 1: GIS Visualization** | Spatial Parcel & Alignment Engine | **Leaflet.js / OpenStreetMap / MapLibre GL** | Lightweight, open-source GIS engine rendering cadastral parcel boundary WKT polygons, corridor centerline alignments, and color-coded readiness layers without vendor lock-in. |
| **Tier 2: Secure Edge Gateway** | Identity, Authorization & PII Protection | **ePramaan / National SSO, ABAC + RBAC, HMAC-SHA256 Tokenization** | Enforces role-based permissions (Ministry, State, CALA, PIU), attribute-based boundaries, and dynamic field-level redaction of Aadhaar/PAN/bank account numbers for public transparency. |
| **Tier 3: Domain Control Plane** | Decoupled State Machines & Lifecycle | **Modern C++17 Native Engine (`dharti_core.dll` / `libdharti_core`)** | Implements asynchronous, non-blocking state progression for Parcels, Claimants, Payments, and R&R with strict invariant validation and multi-agency exception routing. |
| **Tier 3: Statutory Clocks** | Statutory Timeline & SLA Monitor | **C++17 Chrono & Microsecond Precision Timers** | Monitors statutory lapse countdowns (Section 19 12-month limit, Section 25 award deadlines) with automated alerts and predictive delay forecasting. |
| **Tier 4: Trust & Decision Intelligence** | Continuous Frontage (PCI) & Bottleneck Simulator | **C++17 Introsort & Linear Interval Merging ($O(M \log M)$)** | Sub-millisecond mathematical calculation of longest continuous constructible frontage ($\approx 0.024 \text{ ms}$ for 1,000 parcels); ranks blocked parcels by instantaneous $\Delta \text{PCI}$ unlock gain. |
| **Tier 4: Social Safeguards** | SIA "No Family Invisible" Inclusion Engine | **C++17 Set Intersection & Vulnerability Filter** | Audits SIA census households against Section 23 awardees; triggers automated circuit breakers if tenant/vulnerable families are omitted. |
| **Tier 4: Contradiction Engine** | Multi-Agency Cross-Verification | **Deterministic Rule Matrix Engine** | Automatically detects area mismatches between RoR and Cadastral GIS ($> 1\%$), active judicial stay orders, and succession conflicts. |
| **Tier 5: Event Ledger & Audit** | Bitemporal Canonical Event Store | **Immutable Append-Only Log with SHA-256 Cryptographic Hash Chain** | Preserves dual-axis temporal coordinates (Source Time $T_v$ vs Recorded Time $T_t$) guaranteeing Invariant 1 (Zero Silent Overwrites) and Invariant 7 (100% Tamper-Evident Auditability). |
| **Tier 5: Spatial & Structured Persistence** | Relational & Geospatial Storage | **PostgreSQL 16 + PostGIS / SQLite Spatialite (Local)** | Industrial ACID compliance, spatial indexing (R-Tree / GiST), and GeoJSON/WKT query support for nationwide scalability. |
| **Tier 5: Document Repository** | Evidentiary Document Management | **Content-Addressable Storage (MinIO / AWS S3 compliant) with SHA-256 Checksums** | Version-controlled, tamper-evident repository for Gazette notifications, speaking orders, valuation sheets, and geotagged Panchnama field photos. |
| **Tier 6: Federated Adapters** | External Data Ingestion & Normalization | **Modular C++17 Adapters + Zero-Dependency JSON Parser (`json.hpp`)** | Ingests Bhoomi/Bhulekh RoR, BhuNaksha cadastral maps, eCourts CIS court dockets, and PFMS treasury payment advices without external runtime dependencies. |

