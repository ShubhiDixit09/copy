#ifndef DHARTI_MODELS_EVIDENCE_HPP
#define DHARTI_MODELS_EVIDENCE_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <chrono>

namespace dharti {
namespace models {

/**
 * @brief External data source registration.
 */
struct Source {
    std::string source_id;
    std::string code;                // e.g. "PARIVESH", "UP_BHULEKH"
    std::string name;
    std::string source_type;         // "CLEARANCE", "LAND_RECORD", "PAYMENT"
    std::string authority;           // "GOVERNMENT", "MOCK", "THIRD_PARTY"
    int polling_interval_seconds{1800};
    bool is_active{true};
};

/**
 * @brief Source health & observability metric record.
 */
struct SourceHealth {
    std::string source_id;
    std::string source_code;
    std::string last_attempt_at;
    std::string last_success_at;
    uint64_t records_seen{0};
    uint64_t records_changed{0};
    int consecutive_failures{0};
    int64_t current_lag_seconds{0};
    std::string status{"HEALTHY"};   // "HEALTHY", "DEGRADED", "OFFLINE"
    std::string last_error_message;
};

/**
 * @brief Stable external identity for an ingested entity.
 */
struct SourceRecord {
    std::string id;
    std::string source_id;
    std::string source_record_id;    // e.g. "IA/KA/NHA/10482/2026"
    std::string record_type;
    std::string external_id;
    std::string first_seen_at;
    std::string last_seen_at;
};

/**
 * @brief Immutable pointer to an archived raw snapshot in Google Drive.
 */
struct SourceSnapshot {
    std::string snapshot_id;
    std::string source_record_id;
    std::string source_code;
    std::string retrieved_at;
    std::string source_time;
    std::string content_type;        // "application/json", "application/pdf"
    std::string drive_file_id;       // Google Drive file ID
    std::string drive_web_link;
    std::string sha256;              // SHA-256 digest of raw payload
    int http_status{200};
    std::string parser_version{"v1.0.0"};
    std::string schema_version{"v1.0.0"};
    std::string normalized_payload_json;
};

/**
 * @brief Bridge entity between Google Drive evidence vault and Neon DB control plane.
 */
struct EvidenceArtifact {
    std::string id;
    std::string source_id;
    std::string source_record_id;
    std::string artifact_type;       // "CLEARANCE_LETTER", "MUTATION_REGISTER", "PAYMENT_ADVICE"
    std::string drive_file_id;
    std::string drive_url;
    std::string sha256;
    std::string mime_type{"application/json"};
    uint64_t file_size{0};
    std::string source_time;
    std::string retrieved_at;
    std::string parser_version{"v1.0.0"};
    std::string schema_version{"v1.0.0"};
    bool is_current{true};
    std::string supersedes_artifact_id;
    std::string acceptance_status{"PENDING"}; // "PENDING", "ACCEPTED", "QUARANTINED", "REJECTED"
    std::string rejection_reason;
};

/**
 * @brief Canonical workflow event envelope.
 */
struct CanonicalEvent {
    std::string event_id;
    std::string event_type;          // "CLEARANCE_APPROVED", "PARCEL_RECORD_VERIFIED", etc.
    std::string aggregate_type;      // "CLEARANCE", "PARCEL", "PAYMENT"
    std::string aggregate_id;        // e.g. "PARCEL-118", "CLEARANCE-IA-10482"
    std::string source_system;
    std::string source_record_id;
    std::string source_time;
    std::string recorded_time;
    std::string schema_version{"v1.0.0"};
    std::string policy_version{"v1.0.0"};
    std::string correlation_id;
    std::string causation_id;
    std::string payload_json;
    std::string evidence_refs_json;  // JSON array of Drive IDs and hashes
    std::string checksum;
    std::string status{"ACCEPTED"};  // "PENDING", "ACCEPTED", "QUARANTINED", "REJECTED"
};

/**
 * @brief Trapped contradiction or quarantine exception.
 */
struct EvidenceException {
    std::string exception_id;
    std::string code;                // "RULE_5_IMPOSSIBLE_AREA_JUMP", "RULE_1_UNTRUSTED_AUTHORITY"
    std::string severity;            // "LOW", "MEDIUM", "HIGH", "CRITICAL"
    std::string aggregate_type;
    std::string aggregate_id;
    std::string source_record_id;
    std::string reason;
    std::string evidence_ref;        // Google Drive file ID
    bool is_resolved{false};
    std::string created_at;
};

} // namespace models
} // namespace dharti

#endif // DHARTI_MODELS_EVIDENCE_HPP
