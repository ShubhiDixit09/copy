# Chapter 1: Introduction, Scope & Mission

## 1.1 Problem Context
Major infrastructure corridors across India (highways, expressways, dedicated freight corridors, railway expansions, transmission links) suffer severe schedule and cost overruns due to land acquisition bottlenecks. The status quo relies on self-reported dashboard metrics (e.g. "86% land acquired"), which masks underlying operational paralysis:
- Discontinuous, checkerboard parcels prevent physical equipment mobilization.
- Unresolved revenue court stay orders or title disputes halt construction unexpectedly.
- Discrepancies between recorded RoR areas and ground cadastral geometries cause protracted disputes.
- Marginalized, landless, or tenant families enumerated during Social Impact Assessments (SIA) disappear from award schedules, triggering community protests and legal injunctions.
- Disconnected payment instructions leave beneficiaries unpaid despite administrative sanctions.

## 1.2 Core Mission: DHARTI
**DHARTI** is not a replacement land-record database and not merely an executive dashboard. State land registries, revenue courts, finance portals, and environmental clearance platforms remain legally authoritative. DHARTI operates as an **Evidence-Backed National Land Acquisition Control Plane** performing four core jobs:

```
+-----------------------------------------------------------------------------------+
|                                 DHARTI CONTROL PLANE                              |
+---------------------+---------------------+-------------------+-------------------+
|     1. EXCHANGE     |       2. PROVE      |   3. COORDINATE   |     4. DECIDE     |
| Ingest heterogenous | Preserve immutable  | Route statutory   | Compute verified  |
| State & ministry    | source snapshots,   | human workflows   | readiness, PCI,   |
| data via governed   | cryptographic       | with named        | payment lapses &  |
| adapters.           | signatures & rules. | accountability.   | family exclusion. |
+---------------------+---------------------+-------------------+-------------------+
```

## 1.3 Definitions and Standards
- **PCI (Possession Continuity Index)**: The mathematical ratio of the longest uninterrupted, evidence-ready linear constructible frontage to the total corridor length.
- **ULPIN**: Unique Land Parcel Identification Number (14-digit Bhu-Aadhaar).
- **RoR (Record of Rights)**: Authoritative State land ownership and tenancy register.
- **RFCTLARR 2013**: Right to Fair Compensation and Transparency in Land Acquisition, Rehabilitation and Resettlement Act, 2013.
- **Bitemporal Event**: An event preserving both *Source Time* (when the fact occurred in the real world) and *Recorded Time* (when DHARTI captured the evidence).
