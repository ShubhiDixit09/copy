#ifndef DHARTI_SERVICES_EVIDENCE_ENGINE_HPP
#define DHARTI_SERVICES_EVIDENCE_ENGINE_HPP

#include <string>
#include <vector>
#include <memory>
#include "dharti/models/evidence.hpp"
#include "dharti/adapters/parivesh_adapter.hpp"

namespace dharti {
namespace services {

/**
 * @brief Gate evaluation result for an ingested evidence snapshot.
 */
struct GateResult {
    bool is_accepted{false};
    std::string acceptance_status;     // "ACCEPTED", "QUARANTINED", "REJECTED"
    std::string violation_rule;        // e.g. "RULE_5_IMPOSSIBLE_AREA_JUMP"
    std::string reason;
    std::string recommended_event_type;// e.g. "CLEARANCE_APPROVED", "EVIDENCE_QUARANTINED"
    models::EvidenceException generated_exception;
};

/**
 * @brief Change detected between two consecutive snapshots of the same entity.
 */
struct DetectedChange {
    bool has_change{false};
    bool status_changed{false};
    bool area_changed{false};
    std::string old_status;
    std::string new_status;
    double old_area{0.0};
    double new_area{0.0};
    std::string event_type;
    std::string summary;
};

/**
 * @brief 7-Rule Deterministic Evidence Gate & Change Detection Engine.
 * 
 * Enforces:
 *   Rule 1 — Trusted Authority Verification (source.authority == "GOVERNMENT")
 *   Rule 2 — Stable Source Record ID Verification
 *   Rule 3 — Valid Schema Conformance
 *   Rule 4 — Cryptographic SHA-256 Checksum Integrity
 *   Rule 5 — Impossible Jump Detection (e.g. area jump > 10x or negative)
 *   Rule 6 — Identity Conflict Detection
 *   Rule 7 — Cross-Source Contradiction Verification
 */
class EvidenceEngine {
public:
    EvidenceEngine(double area_jump_threshold_multiplier = 10.0);

    /**
     * @brief Evaluates an incoming clearance record against the 7-Rule Evidence Gate.
     * @param record The normalized clearance record.
     * @param authority The authority type of the originating source ("GOVERNMENT", "MOCK", etc.)
     * @param prev_record Optional previous accepted record for change & jump detection.
     */
    GateResult evaluate_clearance(
        const adapters::NormalizedClearanceRecord& record,
        const std::string& authority,
        const adapters::NormalizedClearanceRecord* prev_record = nullptr
    );

    /**
     * @brief Computes field mutations between previous and current snapshots.
     */
    DetectedChange detect_changes(
        const adapters::NormalizedClearanceRecord& current,
        const adapters::NormalizedClearanceRecord& previous
    );

    /**
     * @brief Constructs a canonical workflow event envelope from an accepted or quarantined change.
     */
    models::CanonicalEvent create_event(
        const adapters::NormalizedClearanceRecord& current,
        const GateResult& gate,
        const DetectedChange& change,
        const std::string& drive_file_id,
        const std::string& prev_drive_file_id = ""
    );

private:
    double m_area_jump_threshold_multiplier{10.0};
};

} // namespace services
} // namespace dharti

#endif // DHARTI_SERVICES_EVIDENCE_ENGINE_HPP
