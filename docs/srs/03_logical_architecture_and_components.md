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
