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

---

## 1.4 Scope of Study Table

The following matrix formally defines the operational scope of the National Land Acquisition & Management System study, mapping the complete acquisition lifecycle, statutory provisions, tracked parameters (1 to 15), institutional stakeholders, and DHARTI verification evidence.

| Lifecycle Stage | Statutory Provision / Process | Key Parameters Tracked | Institutional Stakeholders | DHARTI Verification & Evidence Output |
| :--- | :--- | :--- | :--- | :--- |
| **Stage 1: Project Proposal & Alignment** | Preliminary Project Report (PPR) / DPR corridor alignment | • Land proposed (hectares / km)<br>• Project-wise & State-wise alignment<br>• Initial corridor chainage | • Requirers (NHAI / Railways / MoRTH)<br>• Central Ministries<br>• State Nodal Depts | GeoJSON / WKT alignment centerline frozen; Initial baseline PCI established. |
| **Stage 2: Preliminary Notification & Survey** | NH Act Sec 3A / RFCTLARR Sec 11 | • Notifications issued & Gazette dates<br>• Survey numbers & 14-digit ULPINs<br>• Initial RoR landowners list | • Competent Authority (CALA)<br>• District Administration<br>• State Revenue Dept | Notification snapshot hashed; Invariant 1 (Zero Silent Overwrites) enforced. |
| **Stage 3: Social Impact Assessment (SIA)** | RFCTLARR Chapter II (Sec 4–9) | • Affected families enumerated<br>• Displaced vulnerable households<br>• Public hearing records | • SIA Unit / State Government<br>• Gram Sabha / Local Bodies<br>• District Collector | "No Family Invisible" rule audit; unmapped vulnerable families flagged before Section 19. |
| **Stage 4: Objections, Hearing & Scrutiny** | NH Act Sec 3C / RFCTLARR Sec 15 | • Claims & objections lodged<br>• Speaking orders issued<br>• Title/boundary disputes | • Landowners & Tenants<br>• CALA / Revenue Officers<br>• Revenue Courts (RCCMS) | Hearing dockets version-controlled; Active disputes link to parcel state machines. |
| **Stage 5: Final Declaration of Acquisition** | NH Act Sec 3D / RFCTLARR Sec 19 | • Area definitively acquired (Ha/sqm)<br>• Statutory 12-month lapse clock<br>• Land vested in Union / State | • Central Ministry (Gazette)<br>• CALA / District Collector<br>• Project Implementing Agency | Section 19/25 statutory lapse countdown monitored; automated breach alerts dispatched. |
| **Stage 6: Valuation & Award Declaration** | NH Act Sec 3G / RFCTLARR Sec 23 & 26–30 | • Awards declared & sanctioned<br>• Market value, solatium (100%), 12% addl<br>• Total compensation assessed | • CALA / District Collector<br>• Project Director (PIU)<br>• State Finance Dept | Cryptographic valuation formula verification; Award schedule signed and locked. |
| **Stage 7: Direct Financial Disbursement** | PFMS / Treasury / Direct Benefit Transfer (DBT) | • Compensation disbursed vs pending<br>• Bank UTR numbers & credit dates<br>• Failed/rejected payment codes | • Public Financial Management System<br>• State Treasury / Banks<br>• Beneficiaries | Invariant 3 enforcement: Handover blocked if PFMS transaction is pending or failed. |
| **Stage 8: Rehabilitation & Resettlement (R&R)** | RFCTLARR Schedules II & III | • R&R progress & entitlements<br>• Alternative housing allotments<br>• Annuities & subsistence grants | • Administrator R&R<br>• Project Implementing Agency<br>• Affected/Displaced Families | Family-level R&R fulfillment tracking; prevents physical eviction prior to resettlement. |
| **Stage 9: Field Possession & Construction Handover** | NH Act Sec 3E / RFCTLARR Sec 38 | • Possession status (Physical/Legal)<br>• Geotagged Panchnama evidence<br>• Longest continuous frontage (PCI) | • CALA / District Revenue Staff<br>• Concessionaire / Contractor<br>• High Courts (eCourts CIS) | Invariant 2, 4, 5, 6 verified; PCI recalculated to authorize continuous mobilization. |

