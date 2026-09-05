#ifndef DHARTI_CORE_EVENT_STORE_HPP
#define DHARTI_CORE_EVENT_STORE_HPP

#include <string>
#include <vector>
#include <memory>
#include <shared_mutex>
#include <cstdint>
#include <functional>
#include <optional>

namespace dharti {
namespace core {

/**
 * @brief Canonical Bitemporal Event structure.
 * 
 * Implements Invariant 1 (Zero Silent Overwrites) and Invariant 7 (Bitemporal Auditability).
 * Valid Time (Tv): When the event occurred in the physical/statutory world.
 * Transaction Time (Tt): When the control plane recorded and verified the event.
 */
struct BitemporalEvent {
    uint64_t sequence_id{0};
    std::string event_id;
    std::string aggregate_type;    // e.g. "Parcel", "Claimant", "Payment"
    std::string aggregate_id;      // e.g. "P-101", "HH-04", "PMT-883"
    std::string event_type;        // e.g. "PARCEL_INGESTED", "STAY_IMPOSED", "STAY_VACATED", "PAYMENT_CREDITED"
    std::string payload_data;      // Serialized canonical event payload
    std::string valid_time;        // ISO 8601 UTC timestamp of statutory occurrence
    std::string transaction_time;  // ISO 8601 UTC timestamp of system commitment
    std::string recorded_by;       // Authenticated officer / adapter ID
    std::string previous_hash;     // Cryptographic hash linking to parent event
    std::string event_hash;        // Cryptographic integrity hash of this event
};

/**
 * @brief Thread-safe, Append-Only Bitemporal Event Store.
 * 
 * Provides verifiable audit trails, temporal queries, and replay projection.
 */
class BitemporalEventStore {
public:
    BitemporalEventStore();
    ~BitemporalEventStore() = default;

    // Prevent copying to safeguard ledger integrity
    BitemporalEventStore(const BitemporalEventStore&) = delete;
    BitemporalEventStore& operator=(const BitemporalEventStore&) = delete;

    /**
     * @brief Appends an event to the ledger with automatic sequencing and hash chaining.
     * 
     * @param event The bitemporal event to record.
     * @return uint64_t The assigned sequence ID.
     */
    uint64_t append_event(BitemporalEvent event);

    /**
     * @brief Retrieve all events recorded for a specific aggregate.
     */
    std::vector<BitemporalEvent> get_events_for_aggregate(
        const std::string& aggregate_type,
        const std::string& aggregate_id
    ) const;

    /**
     * @brief Query events as of a historical transaction time (Tt).
     * Reconstructs what the system knew at a given historical moment.
     */
    std::vector<BitemporalEvent> query_transaction_as_of(const std::string& max_transaction_time) const;

    /**
     * @brief Query events as of a historical valid time (Tv).
     * Reconstructs the statutory state effective at that point in time.
     */
    std::vector<BitemporalEvent> query_valid_as_of(const std::string& max_valid_time) const;

    /**
     * @brief Cryptographically verifies the unbroken chain of all stored events.
     * @return true if all hashes match and sequence numbers are continuous; false if tampered.
     */
    bool verify_ledger_integrity() const;

    /**
     * @brief Get total number of committed events.
     */
    size_t event_count() const;

    /**
     * @brief Helper hash generator.
     */
    static std::string calculate_hash(const std::string& input);

private:
    mutable std::shared_mutex mutex_;
    std::vector<BitemporalEvent> ledger_;
    std::string latest_hash_{"GENESIS_HASH_DHARTI_CONTROL_PLANE_0000"};
};

} // namespace core
} // namespace dharti

#endif // DHARTI_CORE_EVENT_STORE_HPP
