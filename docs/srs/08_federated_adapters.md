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

