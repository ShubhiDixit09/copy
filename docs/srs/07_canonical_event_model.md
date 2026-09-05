# Chapter 7: Canonical Event & Persistence Model

## 7.1 Event-Driven Architecture
Every action, source update, or decision produces an immutable event. State projections (read models, spatial corridor maps, exception worklists) are derived by replaying or streaming events.

## 7.2 Event Envelope Specification

Every mutation conforms to this standardized schema:

```json
{
  "event_id": "evt_01J6G7N5Q8...",
  "event_type": "ParcelBoundaryResolved | PaymentAckReceived | PossessionRecorded",
  "aggregate_type": "Parcel | Claimant | Payment | RNR | Case",
  "aggregate_id": "agg_ulpin_270102...",
  "source_system": "Adapter_RoR_MH | Adapter_PFMS | Field_PWA",
  "source_timestamp": "2026-09-05T10:15:30Z",
  "recorded_timestamp": 1757088932418,
  "causation_id": "evt_prior_order_...",
  "correlation_id": "proj_delhi_mumbai_pkg1",
  "payload_json": "{\"ulpin\": \"27-01-02-1234\", \"area_sqm\": 1250.0}",
  "metadata_json": "{\"actor_id\": \"usr_collector_01\", \"ip\": \"10.20.1.4\"}",
  "checksum_sha256": "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
}
```

## 7.3 Bitemporal History Model
DHARTI records two temporal dimensions for every fact:
1. **Source Time (Valid Time)**: When the event occurred in the real world or in the authoritative State registry.
2. **Recorded Time (System Time)**: When the DHARTI platform captured and verified the evidence.

This ensures complete audit fidelity: auditors can query "What did the system believe on 15 August 2026 regarding Parcel P-118?" versus "What was the actual legal status on 15 August according to retroactively corrected records?".
