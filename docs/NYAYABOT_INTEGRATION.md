# NyayaBot × DHARTI Integration

## Purpose

NyayaBot is DHARTI's **role-aware, multilingual evidence copilot**. It is an interaction layer over the Evidence Twin and deterministic domain services—not a separate land-record database and not an autonomous decision-maker.

> Anyone may ask. Every answer is limited by identity, purpose, jurisdiction and case relationship.

## Query-to-decision flow

```text
User question
  -> identity, role, case and language context
  -> ShieldAI input/prompt-injection gate
  -> NyayaBot intent and parcel resolver
  -> authorised DHARTI read-only tool
  -> Evidence Twin + versioned policy sources
  -> citation, confidence and privacy gate
  -> grounded answer + Answer Receipt
  -> optional action draft
  -> maker-checker approval outside NyayaBot
```

## Tool contract

| Tool | Reads | Output | NyayaBot may write? |
| :--- | :--- | :--- | :--- |
| Parcel blocker explainer | RoR, cadastral, eCourts, payment and parcel state | Independent open gates with evidence IDs | Draft review task only |
| PCI and unlock simulator | Verified parcel readiness intervals | Current PCI and simulated highest-unlock parcel | No readiness mutation |
| Family inclusion checker | SIA, award and R&R mappings | Missing count and protected categories | Draft inclusion review only |
| Payment reconciler | Award, PFMS advice and bank acknowledgement | Sanctioned vs bank-confirmed state | No payment instruction |
| Audit reconstructor | Bitemporal event ledger | What was known when and from which source | Read only |
| Policy retriever | Effective-dated National and State policy packs | Applicable rule, version and statutory clock | Read only |
| Drafting assistant | Authorised evidence references | Case note, task or grievance draft | Human-reviewed draft only |

The browser prototype implements the first six explanations and safe draft creation using the current in-memory demo data. The old NyayaBot model stack is integrated in the production path described below.

## Role policy

- **CALA / State / Ministry:** project-scoped evidence explanations and review-task drafts.
- **NHAI PIU:** construction, blocker, payment and aggregate inclusion explanations; no entitlement decision.
- **Auditor:** read-only reconstruction and citations; no operational drafts.
- **Affected family:** only self-service payment/R&R/grievance flows after Digital Claim ID verification. Project-wide identities and bank data are hidden.

The prototype's citizen mode intentionally hides the official workspace. Authentication is simulated; production must verify ePramaan identity plus claim-to-case relationship before returning personal data.

## Decision boundaries

NyayaBot must never:

1. decide ownership, compensation eligibility, entitlement or a court outcome;
2. mark a parcel `ConstructionReady`;
3. issue a PFMS payment or silently change an official record;
4. reveal another person's identity, bank reference or grievance;
5. answer a factual/legal question without cited evidence and its `as-of` time.

Every optional write is a uniquely keyed **draft**. Repeating the same action from one Answer Receipt is rejected in the UI, mirroring the original NyayaBot compare-and-swap/idempotency design.

## Production deployment path

```text
DHARTI Web / Citizen Mobile
  -> FastAPI NyayaBot gateway
  -> ShieldAI (PII masking, prompt-injection and output policy)
  -> ReAct tool orchestrator
       -> DHARTI C++ domain APIs
       -> PostgreSQL/PostGIS Evidence Twin
       -> ChromaDB versioned Act -> Section -> State Rule index
  -> optional on-prem Ollama/Gemma explanation and drafting
  -> citation verifier + Answer Receipt
```

- PostgreSQL/PostGIS remains DHARTI's authoritative store.
- ChromaDB stores legal/policy retrieval chunks, never authoritative parcel state.
- SQLite may be retained only for an offline field-device cache or local demo—not as a competing national database.
- Deterministic C++ services calculate PCI, contradictions and state gates. Gemma explains or drafts their outputs.
- English, Hindi and Hinglish use the same evidence IDs, amounts and policy versions.

## Demonstration script

Ask:

> DHARTI reports high acquisition progress. Why is only 41.67% continuously constructible, which parcel unlocks the most work, and are any families unaccounted for?

Use the focused suggestions to demonstrate three receipts:

1. **Explain PCI:** 5.0 km maximum continuous frontage; P-118 simulates 10.0 km / 83.33% if all independent gates are lawfully resolved.
2. **Why P-118 is blocked:** eCourts stay, 15% cadastral/RoR mismatch and unconfirmed compensation.
3. **Missing families:** four SIA-listed vulnerable households are absent from award/R&R mappings; identities are not exposed in the answer.

Then create a **coordinated resolution task draft**. Show that an audit event is added while P-118 remains blocked. This proves that NyayaBot converts evidence into accountable work without replacing the authorised officer.

## Local verification

```bash
node tests/js/test_nyaya_bot.js
```

Serve `web/` using `run_ui.ps1` on Windows or any static HTTP server, then open NyayaBot from the bottom-right corner.
