# Analysis 04: Hybrid Cloud Storage Architecture Feasibility & Effectiveness

**Authors:** DHARTI Engineering Team (SIH 26016)  
**Date:** 2026-09-08  
**Scope:** Neon Serverless PostgreSQL (Control Plane) + Google Drive (Evidence/Object Store) Hybrid Model.

---

## 1. Executive Summary & Problem Formulation
In a production-scale National Land Acquisition & Management System, digital evidence (high-resolution scanned cadastral maps, multi-page High Court injunction orders, gazette notifications, drone survey orthomosaics, and raw API JSON payloads) rapidly totals gigabytes to terabytes. 

Dumping large raw blobs or PDFs directly into a relational database creates several critical anti-patterns:
1. **Bloated Database Storage**: Rapidly exhausts serverless free tiers and escalates I/O cost.
2. **Buffer Cache Thrashing**: Large binary reads evict hot relational indexes and tabular tuples from memory.
3. **Backup & Replication Latency**: Massive dump sizes cripple point-in-time recovery and snapshot replication.

### The Solution: Decoupled Control-Plane / Object-Store Architecture
- **Control Plane (Neon Serverless PostgreSQL)**: Stores structured metadata, 4-tier schemas (Sources, Source Records, Snapshots, Artifacts & Canonical Events), B-tree indexes, PostGIS spatial geometries, SHA-256 digests, and state projections.
- **Evidence Vault (Google Drive Webhook)**: Stores immutable raw evidence files in a structured date-partitioned hierarchy (`DHARTI_EVIDENCE_VAULT/raw/{source}/{YYYY}/{MM}/{DD}/snapshot_XXX.json`). Previous files are never overwritten.

---

## 2. Feasibility & Cost Assessment (₹0 Operational Cost)

| Dimension | Neon Serverless PostgreSQL | Google Drive Webhook Vault | Hybrid Evaluation |
| :--- | :--- | :--- | :--- |
| **Monetary Cost** | ₹0 (Neon Free Tier: 0.5 GiB storage, auto-suspend compute, pooled connections). | ₹0 (Google Apps Script Webhook on personal/institutional Google account with 15 GB quota). | **100% ₹0 Cost Compliance** for prototype and pilot deployments. |
| **API Complexity** | Standard PostgreSQL protocol + PostGIS extension support. | Zero GCP console / cloud billing / OAuth credential configuration needed. | High developer velocity; operates immediately from standard environment variables. |
| **Spatial Capability** | Native PostGIS (`GEOMETRY(Polygon, 4326)`) with GiST indexing. | N/A (Object store). | Enables mathematically exact corridor frontage intersection queries (`ST_Intersects`). |
| **Audit Provenance** | Bitemporal transaction logging with cryptographic SHA-256 chain. | Immutable versioned objects; Google Drive returns permanent `drive_file_id` and `web_link`. | Complete non-repudiation and zero silent overwrite compliance (Invariant 1). |

---

## 3. The 4-Tier Relational Schema in Neon DB

```
+-------------------------------------------------------------------------------+
| LAYER 1: SOURCES & HEALTH (Observability Registry)                            |
|   - sources: (source_id, code, name, authority, polling_interval)             |
|   - source_health: (source_id, status, lag_seconds, last_attempt, failures)   |
+---------------------------------------+---------------------------------------+
                                        | 1:N
+---------------------------------------v---------------------------------------+
| LAYER 2: SOURCE RECORDS (Stable External Identities)                          |
|   - source_records: (id, source_id, source_record_id, record_type)            |
+---------------------------------------+---------------------------------------+
                                        | 1:N
+---------------------------------------v---------------------------------------+
| LAYER 3: SOURCE SNAPSHOTS (Immutable Object Pointers)                         |
|   - source_snapshots: (snapshot_id, source_record_id, drive_file_id, sha256)  |
+---------------------------------------+---------------------------------------+
                                        | 1:N
+---------------------------------------v---------------------------------------+
| LAYER 4: EVIDENCE ARTIFACTS & CANONICAL WORKFLOW EVENTS                       |
|   - evidence_artifacts: (artifact_id, drive_file_id, sha256, acceptance_status)|
|   - workflow_events: (event_id, event_type, aggregate_id, payload, checksum)  |
|   - exceptions: (exception_id, code, severity, reason, evidence_ref)          |
+-------------------------------------------------------------------------------+
```

---

## 4. Empirical Evaluation & Pipeline Verification
In our live verification against the MoEFCC PARIVESH adapter:
1. **Initial Clearance Proposal (`IA/KA/NHA/10482/2026`)**:
   - Normalized in C++17 -> RFC 6234 SHA-256 computed (`58942e6caba8...`).
   - Uploaded to Google Drive Vault -> File ID `1DzOluOK-8OBKaNC8Ri9BTmwBdLNYihlF` returned.
   - Evaluated by 7-Rule Evidence Gate -> `ACCEPTED`.
   - Recorded in Neon DB `sources`, `source_records`, `source_snapshots`, `evidence_artifacts`, and `workflow_events`.
2. **State Transition (Under Process -> Approved)**:
   - Polling Cycle 2 detected mutation -> Emitted canonical event `EVT-651b60418926448c` (`CLEARANCE_APPROVED`), linking both Drive snapshot IDs.
3. **Anomalous Area Jump Test (Circuit Breaker)**:
   - Diversion area jumped from $52.4 \to 5240 \text{ Ha}$ ($100\times$ increase).
   - Rule 5 tripped: Gate classified record as `QUARANTINED`.
   - Event `EVT-8ad71498d4da552c` (`EVIDENCE_QUARANTINED`) logged in Neon DB, exception recorded in `exceptions` table, and `source_health` status updated to `DEGRADED`.
   - **Zero live state corruption occurred.**
