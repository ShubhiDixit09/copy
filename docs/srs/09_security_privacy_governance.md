# Chapter 9: Security, Privacy & Governance

## 9.1 Identity & Access Governance
- **Government SSO**: Integrates with ePramaan / Jan Parichay for single sign-on across district and central ministries.
- **Dual-Model Access Control (RBAC + ABAC)**:
  - *RBAC*: Functional role (Collector, Land Acquisition Officer, Survey Inspector, Auditor).
  - *ABAC*: Attribute constraints (State = MH, District = Pune, Project = Delhi-Mumbai Expressway Package 4, DataClassification = Internal).

## 9.2 Data Protection & Privacy Controls
- **PII Tokenization**: Aadhaar numbers, bank account numbers, and mobile numbers are replaced by opaque pseudonymous UUIDs in domain event streams. Direct PII is stored exclusively in an isolated, encrypted Identity Vault.
- **Field-Level Redaction**: Public transparency views automatically redact names, parcel-level valuations, and personal bank statuses, exposing only generalized cadastral corridors and aggregate percentages.
- **Maker-Checker Protocol**: Final award declarations, payment releases, and possession handovers require dual authorization (Maker = Land Acquisition Officer, Checker = District Collector).

## 9.3 Immutable Audit Trace
Every platform mutation logs:
- `actor_id`: Authenticated user or service identifier.
- `role_and_delegation`: Active role at time of action.
- `timestamp_utc`: Nanosecond-precision recorded time.
- `causation_id`: Triggering statutory notice, court order, or bank advice.
- `action_and_checksum`: SHA-256 hash of payload.
- WORM (Write Once, Read Many) compliance: Audit logs cannot be updated or purged.
