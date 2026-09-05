#ifndef DHARTI_ADAPTERS_ADAPTER_INTERFACE_HPP
#define DHARTI_ADAPTERS_ADAPTER_INTERFACE_HPP

#include <string>
#include <vector>
#include <chrono>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <functional>

namespace dharti {
namespace adapters {

/**
 * @brief Ingestion status enum for external federated records.
 */
enum class IngestionStatus {
    SUCCESS,
    VALIDATION_FAILURE,
    QUARANTINED,
    DUPLICATE_IGNORED
};

/**
 * @brief Common metadata header attached to all ingested federated snapshots.
 */
struct IngestionMetadata {
    std::string source_system;      // e.g. "BHOOMI_KA", "ECOURTS_CIVIL", "PFMS_GOI"
    std::string source_record_id;   // Primary key in the source system
    std::string schema_version;     // Source schema version
    std::string payload_checksum;   // SHA-256 or FNV-1a hash of raw payload
    std::string ingestion_timestamp;// ISO 8601 UTC timestamp
    bool is_quarantined{false};
    std::string quarantine_reason;
};

/**
 * @brief Abstract interface defining the standard contract for federated adapters.
 * 
 * In accordance with Invariant 2 (State Systems Authoritative) and Invariant 1 
 * (Zero Silent Overwrites), adapters normalize heterogeneous state data into 
 * canonical domain structures without modifying authoritative source records.
 */
class IAdapter {
public:
    virtual ~IAdapter() = default;

    /**
     * @brief Unique identifier of the adapter instance.
     */
    virtual std::string adapter_name() const = 0;

    /**
     * @brief Source system type handled by this adapter.
     */
    virtual std::string source_system_code() const = 0;

    /**
     * @brief Current schema version supported.
     */
    virtual std::string supported_schema_version() const = 0;

    /**
     * @brief Computes a deterministic 64-bit FNV-1a hash for payload provenance.
     */
    static std::string compute_hash(const std::string& raw_content) {
        uint64_t hash = 14695981039346656037ULL;
        for (char c : raw_content) {
            hash ^= static_cast<uint8_t>(c);
            hash *= 1099511628211ULL;
        }
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(16) << hash;
        return ss.str();
    }

    /**
     * @brief Get current UTC timestamp formatted as ISO 8601.
     */
    static std::string current_utc_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&in_time_t), "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    }
};

} // namespace adapters
} // namespace dharti

#endif // DHARTI_ADAPTERS_ADAPTER_INTERFACE_HPP
