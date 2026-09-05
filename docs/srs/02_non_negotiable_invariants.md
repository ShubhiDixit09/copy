# Chapter 2: Non-Negotiable System Invariants

The DHARTI platform enforces seven non-negotiable architectural invariants:

### 1. No Status Without Evidence
A dropdown menu, checkbox, or manual status override cannot mark a parcel as *acquired*, *paid*, or *possessed*. Every state transition must be validated against cryptographically signed, provenance-backed evidence (e.g., registered gazette notification, signed compensation receipt, geotagged possession panchnama).

### 2. No Destructive Overwrite
Data corrections, boundary updates, or split/merge events never mutate historical records. The system records an immutable new event and updates the bitemporal timeline, preserving what was known, by whom, and when.

### 3. No Single Project State
A project has no monolithic "status". Project readiness is a derived mathematical function computed from four independent, decoupled state machines: Parcel, Claimant, Payment, and R&R.

### 4. No Silent Conflict Merge
When authoritative sources disagree (e.g., RoR registered titleholder contradicts on-ground claimant, or cadastral map area contradicts RoR area), the system does not pick one arbitrarily or merge silently. It automatically generates a Quarantined Exception Work Item with named owner accountability and statutory SLA timers.

### 5. No AI Adjudication
AI and analytical engines prioritize tasks, explain bottlenecks, and simulate the operational unlock impact of resolving exceptions. Humans (Competent Authority, Collector, Land Acquisition Officer) retain sole statutory authority to decide title, valuation, and eligibility.

### 6. No Central PII Exposure
Personal Identifiable Information (PII) is tokenized and isolated in a purpose-bound identity vault. Domain processing uses pseudonymous identifiers. Public views display only aggregated constructibility metrics and generalized vector geometries.

### 7. No Forced State Migration
State land administrations maintain diverse land systems (e.g. Bhoomi in Karnataka, Dharani in Telangana, Bhulekh in UP, Banglarbhumi in WB). DHARTI provides federated adapters that map diverse State models into a canonical exchange model without requiring States to alter their internal systems.
