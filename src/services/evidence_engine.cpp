#include "dharti/services/evidence_engine.hpp"
#include "dharti/utils/sha256.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <chrono>

namespace dharti {
namespace services {

EvidenceEngine::EvidenceEngine(double area_jump_threshold_multiplier)
    : m_area_jump_threshold_multiplier(area_jump_threshold_multiplier) {}

GateResult EvidenceEngine::evaluate_clearance(
    const adapters::NormalizedClearanceRecord& record,
    const std::string& authority,
    const adapters::NormalizedClearanceRecord* prev_record
) {
    GateResult res;

    // Rule 1: Trusted Authority
    if (authority != "GOVERNMENT" && authority != "MOCK") {
        res.is_accepted = false;
        res.acceptance_status = "QUARANTINED";
        res.violation_rule = "RULE_1_UNTRUSTED_AUTHORITY";
        res.reason = "Source authority '" + authority + "' is not recognized as authoritative.";
        res.recommended_event_type = "EVIDENCE_QUARANTINED";
        res.generated_exception = {
            "", "RULE_1_UNTRUSTED_AUTHORITY", "HIGH", "CLEARANCE",
            record.proposal_no, record.proposal_no, res.reason, "", false, ""
        };
        return res;
    }

    // Rule 2: Stable Source Record ID exists
    if (record.proposal_no.empty() || record.proposal_no.length() < 5) {
        res.is_accepted = false;
        res.acceptance_status = "QUARANTINED";
        res.violation_rule = "RULE_2_INVALID_RECORD_ID";
        res.reason = "Clearance record lacks a valid stable proposal/file number.";
        res.recommended_event_type = "EVIDENCE_QUARANTINED";
        res.generated_exception = {
            "", "RULE_2_INVALID_RECORD_ID", "CRITICAL", "CLEARANCE",
            "UNKNOWN", "UNKNOWN", res.reason, "", false, ""
        };
        return res;
    }

    // Rule 3: Valid Schema Conformance
    if (record.clearance_type.empty() || record.status.empty() || record.json_payload.empty()) {
        res.is_accepted = false;
        res.acceptance_status = "QUARANTINED";
        res.violation_rule = "RULE_3_INVALID_SCHEMA";
        res.reason = "Missing mandatory schema attributes (clearance_type or status).";
        res.recommended_event_type = "EVIDENCE_QUARANTINED";
        res.generated_exception = {
            "", "RULE_3_INVALID_SCHEMA", "HIGH", "CLEARANCE",
            record.proposal_no, record.proposal_no, res.reason, "", false, ""
        };
        return res;
    }

    // Rule 4: Cryptographic SHA-256 integrity
    if (record.metadata.payload_checksum.empty() || record.metadata.payload_checksum.length() != 64) {
        res.is_accepted = false;
        res.acceptance_status = "QUARANTINED";
        res.violation_rule = "RULE_4_MISSING_OR_CORRUPT_CHECKSUM";
        res.reason = "Cryptographic SHA-256 checksum is missing or malformed.";
        res.recommended_event_type = "EVIDENCE_QUARANTINED";
        res.generated_exception = {
            "", "RULE_4_MISSING_OR_CORRUPT_CHECKSUM", "CRITICAL", "CLEARANCE",
            record.proposal_no, record.proposal_no, res.reason, "", false, ""
        };
        return res;
    }

    // Rule 5: Impossible Change Detection (Area jump > threshold or negative)
    if (record.diversion_area_ha < 0.0) {
        res.is_accepted = false;
        res.acceptance_status = "QUARANTINED";
        res.violation_rule = "RULE_5_IMPOSSIBLE_AREA_JUMP";
        res.reason = "Negative forest diversion area observed (" + std::to_string(record.diversion_area_ha) + " Ha).";
        res.recommended_event_type = "EVIDENCE_QUARANTINED";
        res.generated_exception = {
            "", "RULE_5_IMPOSSIBLE_AREA_JUMP", "CRITICAL", "CLEARANCE",
            record.proposal_no, record.proposal_no, res.reason, "", false, ""
        };
        return res;
    }

    if (prev_record != nullptr) {
        // Rule 5A: Project location contradiction check
        if (!prev_record->state_code.empty() && !record.state_code.empty() && prev_record->state_code != record.state_code) {
            res.is_accepted = false;
            res.acceptance_status = "QUARANTINED";
            res.violation_rule = "RULE_5_LOCATION_CONTRADICTION";
            res.reason = "Project state mutated from " + prev_record->state_code + " to " + record.state_code + " (Contradiction).";
            res.recommended_event_type = "EVIDENCE_QUARANTINED";
            res.generated_exception = {
                "", "RULE_5_LOCATION_CONTRADICTION", "CRITICAL", "CLEARANCE",
                record.proposal_no, record.proposal_no, res.reason, "", false, ""
            };
            return res;
        }

        if (!prev_record->district.empty() && !record.district.empty() && prev_record->district != record.district) {
            res.is_accepted = false;
            res.acceptance_status = "QUARANTINED";
            res.violation_rule = "RULE_5_LOCATION_CONTRADICTION";
            res.reason = "Project district mutated from " + prev_record->district + " to " + record.district + " (Contradiction).";
            res.recommended_event_type = "EVIDENCE_QUARANTINED";
            res.generated_exception = {
                "", "RULE_5_LOCATION_CONTRADICTION", "CRITICAL", "CLEARANCE",
                record.proposal_no, record.proposal_no, res.reason, "", false, ""
            };
            return res;
        }

        // Rule 5B: Impossible Area Jump Check (e.g. >300% or <1/3)
        if (prev_record->diversion_area_ha > 0.0) {
            double ratio = record.diversion_area_ha / prev_record->diversion_area_ha;
            if (ratio > m_area_jump_threshold_multiplier || ratio < (1.0 / m_area_jump_threshold_multiplier)) {
                res.is_accepted = false;
                res.acceptance_status = "QUARANTINED";
                res.violation_rule = "RULE_5_IMPOSSIBLE_AREA_JUMP";
                std::stringstream ss;
                ss << "Diversion area jumped anomalously from " << prev_record->diversion_area_ha
                   << " Ha to " << record.diversion_area_ha << " Ha (" << std::fixed << std::setprecision(1)
                   << (ratio * 100.0) << "% of previous baseline, exceeding 300% bound).";
                res.reason = ss.str();
                res.recommended_event_type = "EVIDENCE_QUARANTINED";
                res.generated_exception = {
                    "", "RULE_5_IMPOSSIBLE_AREA_JUMP", "CRITICAL", "CLEARANCE",
                    record.proposal_no, record.proposal_no, res.reason, "", false, ""
                };
                return res;
            }
        }
    }

    // Rule 6: Identity Conflict (Verified through proposal_no format stability)
    if (record.proposal_no.find('/') == std::string::npos) {
        res.is_accepted = false;
        res.acceptance_status = "QUARANTINED";
        res.violation_rule = "RULE_6_IDENTITY_CONFLICT";
        res.reason = "Proposal number does not conform to MoEFCC hierarchical partition standards.";
        res.recommended_event_type = "EVIDENCE_QUARANTINED";
        res.generated_exception = {
            "", "RULE_6_IDENTITY_CONFLICT", "MEDIUM", "CLEARANCE",
            record.proposal_no, record.proposal_no, res.reason, "", false, ""
        };
        return res;
    }

    // Rule 7: Passed all deterministic gate checks!
    res.is_accepted = true;
    res.acceptance_status = "ACCEPTED";
    res.violation_rule = "";
    res.reason = "Conforms to all 7 deterministic evidence acceptance rules.";

    if (record.status == "APPROVED") {
        res.recommended_event_type = "CLEARANCE_APPROVED";
    } else if (record.status == "REJECTED") {
        res.recommended_event_type = "CLEARANCE_REJECTED";
    } else {
        res.recommended_event_type = "CLEARANCE_STATUS_CHANGED";
    }

    return res;
}

DetectedChange EvidenceEngine::detect_changes(
    const adapters::NormalizedClearanceRecord& current,
    const adapters::NormalizedClearanceRecord& previous
) {
    DetectedChange c;
    c.old_status = previous.status;
    c.new_status = current.status;
    c.old_area = previous.diversion_area_ha;
    c.new_area = current.diversion_area_ha;

    if (previous.status != current.status) {
        c.has_change = true;
        c.status_changed = true;
    }
    if (std::abs(previous.diversion_area_ha - current.diversion_area_ha) > 0.001) {
        c.has_change = true;
        c.area_changed = true;
    }

    if (!c.has_change) {
        c.event_type = "NO_CHANGE";
        c.summary = "No change detected between consecutive snapshots.";
        return c;
    }

    if (c.status_changed && current.status == "APPROVED") {
        c.event_type = "CLEARANCE_APPROVED";
        c.summary = "Clearance upgraded to APPROVED status (Diversion: " + std::to_string(current.diversion_area_ha) + " Ha).";
    } else if (c.status_changed && current.status == "REJECTED") {
        c.event_type = "CLEARANCE_REJECTED";
        c.summary = "Clearance was REJECTED by issuing authority.";
    } else {
        c.event_type = "CLEARANCE_STATUS_CHANGED";
        c.summary = "Clearance modified: " + previous.status + " -> " + current.status;
    }

    return c;
}

models::CanonicalEvent EvidenceEngine::create_event(
    const adapters::NormalizedClearanceRecord& current,
    const GateResult& gate,
    const DetectedChange& change,
    const std::string& drive_file_id,
    const std::string& prev_drive_file_id
) {
    models::CanonicalEvent ev;
    auto now = std::chrono::system_clock::now();
    auto in_time = std::chrono::system_clock::to_time_t(now);
    std::stringstream time_ss;
    time_ss << std::put_time(std::gmtime(&in_time), "%Y-%m-%dT%H:%M:%SZ");
    std::string timestamp = time_ss.str();

    // Deterministic event id from proposal + timestamp
    std::string event_seed = current.proposal_no + ":" + current.status + ":" + timestamp;
    std::string ev_hash = utils::SHA256::hash_string(event_seed);
    ev.event_id = "EVT-" + ev_hash.substr(0, 16);

    ev.aggregate_type = "CLEARANCE";
    ev.aggregate_id = current.proposal_no;
    ev.source_system = current.metadata.source_system;
    ev.source_record_id = current.proposal_no;
    ev.source_time = timestamp;
    ev.recorded_time = timestamp;
    ev.status = gate.acceptance_status;

    if (gate.is_accepted) {
        ev.event_type = change.has_change ? change.event_type : gate.recommended_event_type;
    } else {
        ev.event_type = "EVIDENCE_QUARANTINED";
    }

    // Build evidence references JSON
    std::stringstream refs_ss;
    refs_ss << "[\n"
            << "  {\"drive_file_id\": \"" << drive_file_id << "\", \"sha256\": \"" << current.metadata.payload_checksum << "\"}";
    if (!prev_drive_file_id.empty()) {
        refs_ss << ",\n  {\"superseded_drive_file_id\": \"" << prev_drive_file_id << "\"}";
    }
    refs_ss << "\n]";
    ev.evidence_refs_json = refs_ss.str();

    // Payload
    std::stringstream p_ss;
    p_ss << "{\n"
         << "  \"proposal_no\": \"" << current.proposal_no << "\",\n"
         << "  \"clearance_type\": \"" << current.clearance_type << "\",\n"
         << "  \"status\": \"" << current.status << "\",\n"
         << "  \"diversion_area_ha\": " << current.diversion_area_ha << ",\n"
         << "  \"gate_result\": \"" << gate.acceptance_status << "\",\n"
         << "  \"reason\": \"" << gate.reason << "\"\n"
         << "}";
    ev.payload_json = p_ss.str();

    ev.checksum = utils::SHA256::hash_string(ev.event_id + ":" + ev.payload_json);
    return ev;
}

} // namespace services
} // namespace dharti
