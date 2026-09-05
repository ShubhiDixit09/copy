# Chapter 10: Acceptance Scenarios & 4-Minute Walkthrough

## 10.1 Formal Acceptance Criteria (P0 Verification)

- **AT-01 (Idempotent Replay)**: Ingesting an identical batch of Land Record or Payment events creates zero duplicate state transitions or duplicate payments.
- **AT-02 (Non-Destructive Correction)**: Modifying a parcel boundary preserves the previous version and source snapshot in the bitemporal event store.
- **AT-03 (Contradiction Gate)**: An active court stay injunction on Parcel P-118 immediately halts its state progression and prevents it from reaching `ConstructionReady`.
- **AT-04 (Payment Reconciliation)**: A failed bank transaction settlement blocks the payment state from transitioning to `ReceiptConfirmed`.
- **AT-05 (SIA Inclusion Gate)**: Detecting four vulnerable households missing from award/R&R mappings flags a critical exception and prevents project award sign-off.
- **AT-06 (PCI Calculation & Simulation)**: Resolving the highest-priority bottleneck parcel (P-118) recalculates continuous frontage and updates the PCI from 42% to 78% as mathematically predicted.
- **AT-07 (Offline Field Proof)**: Field survey observations recorded offline sync with valid timestamps and witness proofs upon reconnection.
- **AT-08 (Privacy Redaction)**: Public and auditor views display aggregate corridor progress with zero exposed PII.

---

## 10.2 The 4-Minute Demo Script (Page 14 Specification)

| Timeline | Presentation Surface | What the Judges & Evaluators See |
| :--- | :--- | :--- |
| **0:00 - 0:30** | Project Cockpit | Conventional dashboard reports **86% acquired**, but verified constructibility is **61%**, and continuous frontage (**PCI**) is only **42%**. |
| **0:30 - 1:10** | Corridor Gap View | Three parcels break the continuous front. The algorithm identifies **P-118** as the **Highest-Unlock Parcel**. |
| **1:10 - 2:00** | Parcel Evidence Timeline | Clicking P-118 reveals a succession title conflict and an active revenue court stay order preventing work. |
| **2:00 - 2:40** | Family/Entitlement Graph | The SIA Inclusion Engine flags **four livelihood-dependent households** missing from the award schedule. |
| **2:40 - 3:20** | Payment Reconciliation | One compensation payment succeeded, but another has a failed bank acknowledgement (UTR missing). |
| **3:20 - 4:00** | Resolve Simulation | Resolving P-118's court stay and uploading field possession proof increases continuous frontage from 5.0 km to 10.0 km, raising PCI from **42% to 78%**. |
