# Software Requirements Specification (SRS)
## Project Name: DHARTI (Evidence-Backed National Land Acquisition Control Plane)
**Problem Statement ID:** SIH 26016  
**Repository:** [https://github.com/BhaveshBhardwaj/DHARTI](https://github.com/BhaveshBhardwaj/DHARTI)  
**Version:** 1.0.0  
**Status:** Approved Baseline (Living Document - Appendable)  
**Date:** 2026-09-05  

---

## 1. Document Control & Revision History

| Version | Date | Author / Role | Summary of Changes |
| :--- | :--- | :--- | :--- |
| **1.0.0** | 2026-09-05 | Antigravity AI / Bhavesh Bhardwaj | Complete baseline SRS synthesized from SIH 26016 Architecture & Workflow Specification. |
| **1.1.0** | 2026-09-05 | Antigravity AI / Bhavesh Bhardwaj | Added Section 7.1.1: Native C++ Acceleration for PCI engine & established formal CHANGELOG. |
| **1.2.0** | 2026-09-05 | Antigravity AI / Bhavesh Bhardwaj | Fully converted codebase to high-performance pure C++17 modular architecture; removed redundant Python layers. |

---

## 2. Executive Summary & Problem Context

### 2.1 Problem Overview
Major infrastructure corridors (highways, rail, expressways, transmission lines) in India face extensive delays due to fragmented land acquisition processes. Traditional management systems rely on self-reported status percentages from dashboards, where land is reported as "85% acquired" but constructibility remains blocked because scattered, discontinuous parcels remain unpossessed, contested in courts, unpaid, or tied up in unaddressed Rehabilitation & Resettlement (R&R) issues.

### 2.2 Project Mission: DHARTI
**DHARTI** is not a replacement land-record database and not merely a reporting dashboard. State land, registration, revenue court, finance, and environmental clearance systems remain authoritative. DHARTI serves as an **Evidence-Backed National Land Acquisition Control Plane** performing four core jobs:
1. **Exchange**: Ingest heterogeneous State and ministry data through governed, schema-validated adapters.
2. **Prove**: Preserve immutable source evidence, historical snapshots, digital signatures, and cross-source contradictions.
3. **Coordinate**: Route long-running statutory and human workflows with explicit named accountability and SLA tracking.
4. **Decide**: Compute verified readiness, uninterrupted corridor continuity, payment reconciliation exceptions, and family exclusion risks.

---

## 3. Non-Negotiable System Invariants

1. **No Status Without Evidence**: A dropdown or manual flag cannot transition a parcel to acquired, paid, or possessed. Every transition requires cryptographically verifiable or source-backed evidence.
2. **No Destructive Overwrite**: Corrections, splits, or merges create new immutable events and bitemporal states, fully preserving previous facts.
3. **No Single Project State**: Parcel, claimant, payment, and R&R states advance independently along decoupled state machines. Project readiness is a derived mathematical function, never a monolithic status.
4. **No Silent Conflict Merge**: Contradictory authoritative sources (e.g., RoR vs Cadastral Map vs Court Injunction) automatically create an explicit exception work item.
5. **No AI Adjudication**: AI models prioritize, explain delays, and simulate impact; humans (Collector, LAA, competent authorities) decide title, compensation, and statutory eligibility.
6. **No Central PII Exposure**: All personal identifying data uses tokenization, purpose-bound access control (ABAC), and field-level redaction. Public views see only aggregates.
7. **No Forced State Migration**: Adapters translate diverse State land record formats (Bhoomi, Dharani, Banglarbhumi, Bhulekh, etc.) into a canonical exchange model without altering State workflows.

---

## 4. Logical System Architecture & Modular Boundaries

DHARTI is organized as a clean, event-driven modular monolith with decoupled components:

```
                                +-----------------------------------+
                                |     Decision Surfaces & Clients   |
                                | National Cockpit | District Bench |
                                | Field PWA (Offline) | Citizen App |
                                +-----------------+-----------------+
                                                  |
                                                  v
+---------------------------------------------------------------------------------------------------+
|                                  API Gateway & Security Edge                                      |
|             ePramaan / SSO  |  RBAC + ABAC  |  Field Redaction  |  Audit Interceptor              |
+-------------------------------------------------+-------------------------------------------------+
                                                  |
                                                  v
+---------------------------------------------------------------------------------------------------+
|                                      Domain Control Plane                                         |
|  +--------------------+  +--------------------+  +--------------------+  +--------------------+   |
|  | Project/Alignment  |  | Parcel GIS Lineage |  | Workflow & Clocks  |  | Evidence / Docs    |   |
|  +--------------------+  +--------------------+  +--------------------+  +--------------------+   |
|  +--------------------+  +--------------------+  +--------------------+  +--------------------+   |
|  | Party & Interests  |  | Award/Entitlement  |  | Payment Reconcile  |  | R&R & Grievance    |   |
|  +--------------------+  +--------------------+  +--------------------+  +--------------------+   |
+-------------------------------------------------+-------------------------------------------------+
                                                  |
                                                  v
+---------------------------------------------------------------------------------------------------+
|                                  Trust & Decision Intelligence                                    |
|  +---------------------------+  +---------------------------+  +-------------------------------+  |
|  | Contradiction Engine      |  | PCI & Unlock Simulator    |  | No-Family-Invisible Rule      |  |
|  | (Cross-source conflict)   |  | (Chainage continuity)     |  | (SIA-to-Award Reconciliation) |  |
|  +---------------------------+  +---------------------------+  +-------------------------------+  |
+-------------------------------------------------+-------------------------------------------------+
                                                  |
                                                  v
+---------------------------------------------------------------------------------------------------+
|                                  Event Store & Persistence Layer                                  |
|        Canonical Event Log  |  PostgreSQL + PostGIS  |  Versioned S3/MinIO Object Store           |
+-------------------------------------------------+-------------------------------------------------+
                                                  ^
                                                  |
+-------------------------------------------------+-------------------------------------------------+
|                                    Federated Source Adapters                                      |
|    Land Record / Cadastral    |    eCourts / Revenue Courts    |    PFMS / Bank Gateway           |
+---------------------------------------------------------------------------------------------------+
```

### 4.1 Physical Codebase Modular Structure (Pure C++17)
```
dharti/
├── Makefile                # Build automation (compile shared lib & test runner)
├── build.ps1               # One-click PowerShell build and verification script
├── CHANGELOG.md            # Living changelog tracking all revisions
├── docs/                   # Specifications, architecture manuals, diagrams
│   ├── SRS.md              # Living Software Requirements Specification
│   ├── ARCHITECTURE.md     # Architecture overview and design patterns
│   └── resources/          # Source PDFs, posters, reference schemas
├── include/                # Public C++ Header Specifications
│   └── dharti/
│       ├── config/         # AppConfig environment loader (settings.hpp)
│       ├── core/           # Base abstractions (base.hpp, event_envelope.hpp)
│       ├── models/         # Domain entities (parcel.hpp, claimant.hpp, payment.hpp, contradiction.hpp)
│       ├── services/       # Domain business logic (contradiction_engine.hpp, pci_engine.hpp, sia_inclusion_engine.hpp)
│       └── utils/          # Structured logger utility (logger.hpp)
├── src/                    # C++ Implementations
│   ├── config/             # Config implementation (settings.cpp)
│   ├── services/           # Service implementations (pci_engine.cpp)
│   ├── utils/              # Logger implementation (logger.cpp)
│   └── native/             # Core computational engines & exported C ABI (dharti_c_api.cpp, pci_engine.cpp, etc.)
├── tests/                  # Automated C++ Test Suites
│   └── cpp/                # Unit & integration test runners (test_main.cpp)
├── .gitignore              # Clean ignore rules
└── README.md               # Quickstart and project navigation
```

---

## 5. End-to-End Workflow Stages

The platform coordinates eight distinct, evidence-gated statutory stages:

```
[1. INITIATE] ──> [2. VERIFY] ──> [3. NOTIFY] ──> [4. RESOLVE]
                                                       │
                                                       v
[8. ASSURE]   <── [7. POSSESS]<── [6. DELIVER]<── [5. AWARD]
```

### Stage 1 - Initiate
- **Inputs**: Project purpose, requiring authority, approved alignment corridor, budget envelope, applicable statutory regime (e.g. RFCTLARR 2013).
- **Actions**: Assign immutable project case ID; load jurisdiction policy pack; import alignment geometries; partition into candidate parcels; initiate SIA and financial cases.
- **Gate**: Authority, purpose, valid corridor geometry, and required proposal documents verified.
- **Output**: Submitted project version and candidate parcel inventory.

### Stage 2 - Verify
- **Inputs**: Land records (RoR), registration records, cadastral vector polygons, revenue court registries.
- **Actions**: Cross-check title, area, and ownership; compute spatial overlay; build parcel lineage; enumerate affected households and vulnerable groups; detect discrepancies.
- **Gate**: Source confidence verification. Critical conflicts create quarantined exception work items without halting unaffected parcels.
- **Output**: Evidence confidence profile and verified parcel inventory.

### Stage 3 - Notify
- **Inputs**: Verified parcel extract and boundary descriptions.
- **Actions**: Freeze notified geometry snapshot; issue statutory notice; publish multilingual citizen-accessible extracts; start statutory clocks (e.g., 60-day objection period); track service and re-service.
- **Gate**: Notice delivery confirmation and immutable gazette notification record.
- **Output**: Formal statutory notice active; objection/hearing channels opened.

### Stage 4 - Resolve
- **Inputs**: Public objections, court stay notices, heir/succession claims, boundary disputes.
- **Actions**: Conduct hearings; link eCourt/RCCMS case dockets; record speaking orders; reconcile parcel boundaries; update ownership without overwriting historical records.
- **Gate**: All blocking objections for a parcel resolved with recorded speaking order or court vacation.
- **Output**: Legally clean, declared parcel records ready for award valuation.

### Stage 5 - Award
- **Inputs**: Valuation matrix, market value assessments, solatium, asset enumeration, R&R entitlement plan.
- **Actions**: Freeze acquired vs residual geometry; generate signed award version; create financial obligation control totals; link R&R entitlements.
- **Gate**: Mandatory check: No high-risk unmatched household from SIA and no unresolved boundary/title dispute.
- **Output**: Signed award package and legal compensation liability.

### Stage 6 - Deliver
- **Inputs**: Beneficiary bank accounts, PFMS payment instructions, court deposit mechanisms.
- **Actions**: Dispatch payment instruction; ingest PFMS response; capture bank acknowledgements; verify beneficiary credit or lawful section 77/court deposit.
- **Gate**: Reconciliation verification: Zero unverified payment flags.
- **Output**: Legally completed compensation record.

### Stage 7 - Possess
- **Inputs**: Field possession team deployment, panchnama/memorandum.
- **Actions**: Offline field survey; geotagged/timestamped boundary photos; witness signatures; upload signed possession memorandum.
- **Gate**: Dual verification: Legal award paid + physical field possession proof captured.
- **Output**: Physical possession verified; parcel flagged ready for construction.

### Stage 8 - Assure
- **Inputs**: Long-term R&R milestone tracking, post-possession audit, environmental clearance conditions.
- **Actions**: Verify R&R outcome sustainability (e.g. 3-month and 6-month livelihood checkpoints); compile complete immutable audit trail.
- **Gate**: Audit sign-off; complete retention schedule enforcement.
- **Output**: Formal handover of continuous constructible frontage to concessionaire/contractor.

---

## 6. Decoupled State Machines & Derived Readiness

### 6.1 State Machine Matrix

| Entity | State Progression | Evidence Required to Advance |
| :--- | :--- | :--- |
| **Parcel** | `Candidate` ➔ `Notified` ➔ `Awarded` ➔ `PossessionVerified` ➔ `ConstructionReady` | Freezing geometry, signed gazette, signed award, geotagged field panchnama. |
| **Claimant** | `ObservedInSIA` ➔ `IdentityResolved` ➔ `InterestVerified` ➔ `EntitlementMapped` ➔ `Resolved` | Aadhaar/e-KYC token, RoR/tenancy proof, award entitlement link. |
| **Payment** | `ObligationCreated` ➔ `Sanctioned` ➔ `PFMSInstructed` ➔ `BankAcknowledged` ➔ `ReceiptConfirmed` | Financial sanction, PFMS transaction reference, bank credit ACK / lawful court deposit. |
| **R&R** | `PlanApproved` ➔ `ServiceDelivered` ➔ `FamilyVerified` ➔ `RelocationSafe` ➔ `OutcomeSustained` | Housing allotment, physical verification, livelihood audit confirmation. |

### 6.2 Derived Readiness Rule
A project **never** possesses a single manually editable status. Overall project progress is derived mathematically:
$$\text{Verified Progress} = \frac{\sum \text{Area}(\text{Parcels where State} = \text{ConstructionReady})}{\text{Total Project Alignment Area}}$$

---

## 7. Core Deterministic Algorithms

### 7.1 Possession Continuity Index (PCI)
The primary metric of DHARTI is **verified continuous constructible frontage**, not raw gross area acquired.
1. Intersect project corridor centerline with parcel boundaries to produce ordered chainage intervals: $[c_0, c_1], [c_1, c_2], \dots, [c_{n-1}, c_n]$.
2. For each interval $i$, evaluate boolean function $\text{ready}(i)$:
   $$\text{ready}(i) = (\text{ParcelState} = \text{ConstructionReady}) \land (\text{PaymentState} = \text{ReceiptConfirmed}) \land (\text{R&RState} \in \{\text{RelocationSafe}, \text{OutcomeSustained}\}) \land (\text{ActiveCourtStay} = \text{False})$$
3. Compute continuous runs of adjacent ready intervals. Let $L_{\text{continuous}}^{\max}$ be the length of the longest contiguous ready run.
4. Calculate PCI:
   $$\text{PCI} = \frac{L_{\text{continuous}}^{\max}}{L_{\text{total alignment}}}$$
5. **Unlock Simulation**: For each blocked parcel $p$, simulate flipping $\text{ready}(p) = \text{True}$. Compute $\Delta L_{\text{continuous}}^{\max}(p)$. Rank parcels by $\Delta L_{\text{continuous}}^{\max}(p)$ to direct Collector/LAA focus to the parcel with highest operational unlock value.

#### 7.1.1 High-Performance C++ Native Engine
- For large-scale national corridors spanning hundreds of kilometers and tens of thousands of cadastral parcels, the interval merging and exhaustive $O(N)$ unlock simulation must achieve sub-millisecond latency.
- Performance-critical mathematical and geometric routines are implemented in **modern C++ (C++17/C++20)** within `src/native/` and exposed via an `extern "C"` ABI.
- The platform provides dual-mode execution:
  - **Native C++ Engine**: High-throughput vectorized computation via dynamic library bindings.
  - **Python Fallback Engine**: Pure Python implementation ensuring 100% testability and portability across environments without native toolchains.

### 7.2 Evidence Confidence & Contradiction Engine
Detects deterministic discrepancies across authoritative records:
- **Title Mismatch**: RoR registered owner $\ne$ field survey claimant.
- **Area/Geometry Discrepancy**: Cadastral map polygon area $\ne$ RoR recorded area by $> \pm 1\%$.
- **Active Encumbrance / Injunction**: Active stay order in eCourts or RCCMS matching parcel survey number.
- **Action**: When a contradiction is flagged, the engine automatically creates a Quarantined Exception Work Item assigned to the responsible official with SLA timer. The parcel's gate is blocked, but adjacent independent parcels continue unaffected.

### 7.3 No Family Invisible Rule (SIA Safeguard)
Ensures vulnerable, landless, or tenant households enumerated during Social Impact Assessment (SIA) are never bypassed:
$$\text{Affected Universe} = \text{SIA Observations} \cup \text{Field Surveys} \cup \text{Recorded Tenancy} \cup \text{FRA Rights Holders}$$
Every household in the Affected Universe must link to an explicit, signed award record, entitlement disposition, or reviewed lawful exemption. A status of "Not Eligible" is acceptable if supported by a speaking order; disappearing from the records is an unbypassable system violation.

---

## 8. Canonical Event Envelope Specification

Every mutation across internal modules or external adapters produces an immutable event conforming to this JSON schema:

```json
{
  "event_id": "evt_01J6G7N5Q8...",
  "event_type": "ParcelBoundaryResolved | PaymentAckReceived | PossessionRecorded",
  "aggregate_type": "Parcel | Claimant | Payment | RNR | Case",
  "aggregate_id": "agg_ulpin_270102...",
  "source_system": "Adapter_RoR_MH | Adapter_PFMS | Field_PWA",
  "source_timestamp": "2026-09-05T10:15:30Z",
  "recorded_timestamp": "2026-09-05T10:15:32.418Z",
  "causation_id": "evt_prior_order_...",
  "correlation_id": "proj_delhi_mumbai_pkg1",
  "payload": {
    "ulpin": "27-01-02-1234-5678",
    "status": "PossessionVerified",
    "evidence_hash": "sha256:e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
    "witnesses": ["Revenue Inspector A", "Village Patwari B"]
  },
  "metadata": {
    "actor_id": "usr_collector_01",
    "ip_address": "10.20.1.4",
    "policy_pack_version": "MH_RFCTLARR_v2.1"
  }
}
```

---

## 9. Integration Adapters Specification

The system specifies three foundational mock/production adapters:
1. **Land Record & Cadastral Adapter**:
   - Ingests land ownership records (RoR), ULPIN identifiers, and GeoJSON cadastral vector polygons.
   - Emits: `ParcelIngested`, `ParcelGeometryUpdated`, `OwnershipRecorded`.
2. **Court & Litigation Adapter (eCourts / RCCMS)**:
   - Queries revenue court and judicial case dockets for case filing, stay injunctions, and disposed speaking orders.
   - Emits: `CaseLinked`, `InjunctionImposed`, `InjunctionVacated`.
3. **Finance & Banking Gateway (PFMS / State Treasury)**:
   - Dispatches electronic compensation payment instructions; ingests PFMS status files and bank NEFT/RTGS transaction settlement confirmations.
   - Emits: `PaymentInstructionDispatched`, `BankAckReceived`, `PaymentReconciled`, `PaymentFailed`.

---

## 10. Security, Privacy & Public Transparency

1. **Identity & Access**: ePramaan / Government SSO compatibility with OAuth2/OIDC, Multi-Factor Authentication for administrative/Collector roles.
2. **Role-Based & Attribute-Based Access Control (RBAC + ABAC)**: Scoped by Jurisdiction (State/District/Taluka), Project ID, and Data Sensitivity Level.
3. **PII Tokenization & Vault**: Direct names, bank accounts, and identity tokens are stored in a secure encrypted vault. Domain processing relies on pseudonymous identifiers.
4. **Public Redaction**: Public views display anonymized, aggregated corridor constructibility and entitlement status without individual personal information.
5. **Maker-Checker Protocol**: Mandatory dual authorization for award finalization, payment instruction issuance, and possession confirmation.

---

## 11. Acceptance Test Scenarios (P0/P1 Criteria)

- **AT-01 (Idempotent Replay)**: Replaying the identical adapter transaction batch creates no duplicate events or payment instructions.
- **AT-02 (Immutability)**: Correcting a parcel boundary creates a superseding version; the prior geometry and checksum remain intact in the event log.
- **AT-03 (Contradiction Gate)**: An active court stay on Parcel P-118 immediately blocks its state from reaching `ConstructionReady` and reduces overall PCI.
- **AT-04 (Payment Reconciliation)**: A simulated failed bank acknowledgement blocks the payment state from transitioning to `ReceiptConfirmed`.
- **AT-05 (SIA Inclusion Gate)**: Detecting 4 SIA households missing from award schedules triggers a critical exception and prevents award sign-off.
- **AT-06 (PCI Calculation & Simulation)**: Resolving the highest-priority bottleneck parcel recalculates continuous frontage and updates PCI exactly as predicted.
- **AT-07 (Field Proof Offline Sync)**: Offline surveyor submission queues in IndexedDB and commits with valid geotag and timestamp on reconnect.
