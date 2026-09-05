# Software Requirements Specification (SRS)
## Project Name: DHARTI (Evidence-Backed National Land Acquisition Control Plane)
**Problem Statement ID:** SIH 26016  
**Repository:** [https://github.com/BhaveshBhardwaj/DHARTI](https://github.com/BhaveshBhardwaj/DHARTI)  
**Version:** 1.3.0-modular  
**Status:** Approved Living Specification  
**Architecture:** Pure Modern C++17 Modular Control Plane  

---

## Document Control & Chapter Navigation

This specification is organized into modular chapters, each detailing a specific layer of the DHARTI system architecture:

| Chapter | Document | Topic & Purpose |
| :--- | :--- | :--- |
| **00** | [Index & Executive Summary](00_index.md) | Document control, revision history, and problem overview. |
| **01** | [Introduction & Scope](01_introduction_and_scope.md) | Purpose, definitions, four core jobs (Exchange, Prove, Coordinate, Decide). |
| **02** | [Non-Negotiable Invariants](02_non_negotiable_invariants.md) | Seven architectural rules (No status without evidence, no destructive overwrite, etc.). |
| **03** | [Logical Architecture](03_logical_architecture_and_components.md) | Federated multi-tier architecture and component responsibilities. |
| **04** | [Statutory Workflow Stages](04_workflow_stages.md) | End-to-end 8 statutory stages from Initiate to Assure. |
| **05** | [Decoupled State Machines](05_decoupled_state_machines.md) | Independent state lifecycles for Parcel, Claimant, Payment, and R&R. |
| **06** | [Deterministic Algorithms](06_deterministic_algorithms.md) | PCI mathematical formula, unlock simulation, contradiction engine, and SIA rules. |
| **07** | [Canonical Event Model](07_canonical_event_model.md) | Bitemporal event envelope, SHA-256 provenance, and immutable event store. |
| **08** | [Federated Source Adapters](08_federated_adapters.md) | Land Record (RoR/Cadastral), Court (eCourts/RCCMS), and Finance (PFMS) adapter contracts. |
| **09** | [Security, Privacy & Governance](09_security_privacy_governance.md) | PII tokenization, ABAC/RBAC, field-level redaction, and audit logging. |
| **10** | [Acceptance & Demo Walkthrough](10_acceptance_and_demo_scenarios.md) | Acceptance criteria (AT-01 to AT-08) and exact 4-minute demo scenario. |

---

## Document Revision History

| Version | Date | Author / Role | Summary of Changes |
| :--- | :--- | :--- | :--- |
| **1.0.0** | 2026-09-05 | Antigravity AI / Bhavesh Bhardwaj | Initial monolithic baseline synthesized from SIH 26016 specification. |
| **1.1.0** | 2026-09-05 | Antigravity AI / Bhavesh Bhardwaj | Added Native C++ Acceleration specifications (Section 7.1.1). |
| **1.2.0** | 2026-09-05 | Antigravity AI / Bhavesh Bhardwaj | Converted codebase structure to 100% pure C++17 modular architecture. |
| **1.3.0** | 2026-09-05 | Antigravity AI / Bhavesh Bhardwaj | Modularized SRS into multi-chapter directory format with federated adapter specifications. |
