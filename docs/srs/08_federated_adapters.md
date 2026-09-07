# Chapter 8: Federated Source Adapters

## 8.1 Federated Adapter Contract
DHARTI integrates with authoritative external platforms via non-intrusive adapters. Each adapter satisfies:
1. **Schema Validation & Versioning**: Rejects malformed records at the inbound edge.
2. **Idempotency**: Duplicate inbound messages produce zero duplicate state transitions or duplicate payments.
3. **Cryptographic Checksumming**: Emits SHA-256 payload checksums for audit validation.
4. **Quarantine Dead-Letter Handling**: Records with validation or signature failures enter a quarantine queue with an explicit reason code.

## 8.2 The Three Baseline Adapters

### 1. Land Record & Cadastral GIS Adapter
- **External Counterparts**: State RoR systems (Bhoomi, Bhulekh, Banglarbhumi, Dharani) + Cadastral GIS (BhuNaksha).
- **Ingestion Mode**: Periodic signed batches or Webhook / REST pull.
- **Inbound Data**: ULPIN, survey number, recorded owner name, tenancy details, cadastral vector polygon coordinates, area in square meters.
- **Events Emitted**: `ParcelIngested`, `ParcelGeometryUpdated`, `OwnershipRecorded`.

### 2. Court & Litigation Adapter
- **External Counterparts**: eCourts National Judicial Data Grid (NJDG) + State Revenue Court Computerized Management Systems (RCCMS).
- **Ingestion Mode**: Automated nightly sync or webhook on hearing listing.
- **Inbound Data**: Case CNR number, court bench, petitioner/respondent names, interim stay order flag, disposed date, speaking order PDF reference.
- **Events Emitted**: `CaseLinked`, `InjunctionImposed`, `InjunctionVacated`.

### 3. Finance & Banking Gateway Adapter
- **External Counterparts**: Public Financial Management System (PFMS), State Treasuries, National Automated Clearing House (NACH) / Core Banking.
- **Ingestion Mode**: Bidirectional API / secure electronic file transfer.
- **Outbound**: Sanctioned payment advice with ULPIN, beneficiary account token, and amount.
- **Inbound**: PFMS ACK reference, Bank UTR number, transaction settlement date, credit confirmation, or return failure code.
- **Events Emitted**: `PaymentInstructionDispatched`, `BankAckReceived`, `PaymentReconciled`, `PaymentFailed`.

## 8.3 Generalized Real Data Ingestion Engine

To support operational deployment without external server dependencies, DHARTI provides a generalized, file-driven ingestion interface powered by a zero-dependency C++17 recursive-descent JSON parser (`include/dharti/utils/json.hpp`).

### Ingestion Contract & Schemas

| Source Interface | File Path | Standard Schema Elements |
| :--- | :--- | :--- |
| **Project Specification** | `data/projects/*.json` | `projectId`, `name`, `corridorLengthKm`, `gazetteNotification` |
| **Bhoomi Land Records (RoR)** | `data/ror/*.json` | `records[]`: `parcelId`, `ulpin`, `khataNumber`, `ownerName`, `areaSqm`, `encumbranceNotes` |
| **Cadastral GIS (BhuNaksha)** | `data/cadastral/*.json` | `parcels[]`: `parcelId`, `ulpin`, `chainageStartKm`, `chainageEndKm`, `wktPolygon` |
| **eCourts CIS Dockets** | `data/ecourts/*.json` | `cases[]`: `cnrNumber`, `parcelId`, `courtName`, `isStayGranted`, `orderSummary` |
| **PFMS Treasury Mandates** | `data/pfms/*.json` | `advices[]`: `parcelId`, `ulpin`, `beneficiaryName`, `amountInr`, `state`, `bankUtr` |
| **RFCTLARR SIA Census** | `data/sia/*.json` | `households[]`: `householdId`, `headOfHousehold`, `isVulnerable`, `awardStatus` |

### Generalization for Arbitrary User Inputs
Any external system or user can provide custom files formatted according to the above JSON contracts. The CLI tool `bin/dharti_cli.exe` ingests them via dynamic CLI flags:
```bash
./bin/dharti_cli.exe \
  --project data/projects/nhai_corridor_12km.json \
  --ror data/ror/karnataka_revenue_ror.json \
  --cadastral data/cadastral/bhoomi_cadastral_parcels.json \
  --court data/ecourts/high_court_dockets.json \
  --pfms data/pfms/treasury_payment_advices.json \
  --sia data/sia/sia_household_census.json
```
The federated adapters parse, validate, and convert inbound raw JSON nodes into strongly-typed domain structures, calculate cryptographic hashes, and register them directly with the `WorkflowCoordinator`.

## 8.4 MoEFCC PARIVESH Clearance Adapter & 7-Rule Evidence Gate

### 1. Architectural Scope & Decoupled Vault Pattern
To integrate Ministry of Environment, Forest and Climate Change (MoEFCC) statutory clearances into the corridor readiness lifecycle, DHARTI implements the **PARIVESH Single-Window Clearance Adapter** (`include/dharti/adapters/parivesh_adapter.hpp`).
- **Google Drive Object Vault**: All raw JSON snapshots and PDF approval letters are preserved immutably in partitioned folders (`DHARTI_EVIDENCE_VAULT/raw/parivesh/{YYYY}/{MM}/{DD}/snapshot_XXX.json`) and never overwritten.
- **Neon Control-Plane Relational Ledger**: Stores machine pointers (`drive_file_id`), SHA-256 digests, metadata, and state transitions.

### 2. The 7 Non-Negotiable Evidence Acceptance Rules
Before any external clearance snapshot modifies state or triggers canonical workflow events, it is evaluated by the deterministic **Evidence Engine** (`include/dharti/services/evidence_engine.hpp`):

1. **Rule 1 — Trusted Authority Verification**: Originating authority must equal `GOVERNMENT` or authenticated `MOCK`. Untrusted sources are trapped in `QUARANTINE`.
2. **Rule 2 — Stable Source Record ID**: Enforces valid, stable proposal/file numbers (e.g. `IA/KA/NHA/10482/2026`).
3. **Rule 3 — Valid Schema Conformance**: All mandatory attributes (clearance type, diversion area, issuing authority) must be present and well-formed.
4. **Rule 4 — Cryptographic Hash Integrity**: Verifies presence and format of RFC 6234 SHA-256 digest.
5. **Rule 5 — Impossible Jump Detection**: Traps anomalies (e.g. forest diversion area jumping $> 1000\%$ or negative area) into `QUARANTINED` with `RULE_5_IMPOSSIBLE_AREA_JUMP`.
6. **Rule 6 — Identity Conflict Detection**: Validates hierarchical partition formatting across states and projects.
7. **Rule 7 — Cross-Source Contradiction**: Validates clearance footprint against corridor alignment and RoR boundary definitions.

### 3. Canonical Events Emitted
- `CLEARANCE_STATUS_CHANGED`: Emitted upon accepted status or mitigation condition modifications.
- `CLEARANCE_APPROVED`: Emitted upon official grant of Stage-1/Stage-2 Forest Clearance or Environmental Clearance.
- `EVIDENCE_QUARANTINED`: Emitted when any of the 7 rules trip, preventing state pollution and alerting compliance teams.

