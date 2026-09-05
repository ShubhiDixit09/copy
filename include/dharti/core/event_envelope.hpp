#ifndef DHARTI_CORE_EVENT_ENVELOPE_HPP
#define DHARTI_CORE_EVENT_ENVELOPE_HPP

#include <string>
#include <cstdint>

namespace dharti {
namespace core {

/**
 * @brief Standardized Canonical Event Envelope conforming to SIH 26016 Section 8.
 * Immutable record of any state mutation or external source event.
 */
struct EventEnvelope {
    std::string event_id;
    std::string event_type;
    std::string aggregate_type;
    std::string aggregate_id;
    std::string source_system;
    std::string source_timestamp;
    int64_t recorded_timestamp_ms;
    std::string causation_id;
    std::string correlation_id;
    std::string payload_json;
    std::string metadata_json;
    std::string checksum_sha256;
};

} // namespace core
} // namespace dharti

#endif // DHARTI_CORE_EVENT_ENVELOPE_HPP
