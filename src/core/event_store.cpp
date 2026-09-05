#include "dharti/core/event_store.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <mutex>

namespace dharti {
namespace core {

std::string BitemporalEventStore::calculate_hash(const std::string& input) {
    uint64_t hash = 14695981039346656037ULL;
    for (char c : input) {
        hash ^= static_cast<uint8_t>(c);
        hash *= 1099511628211ULL;
    }
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << hash;
    return ss.str();
}

BitemporalEventStore::BitemporalEventStore() {
    // Initialized with genesis hash
}

uint64_t BitemporalEventStore::append_event(BitemporalEvent event) {
    std::unique_lock<std::shared_mutex> lock(mutex_);

    event.sequence_id = ledger_.size() + 1;
    event.previous_hash = latest_hash_;

    // Compute cryptographic event hash linking previous hash
    std::stringstream payload_builder;
    payload_builder << event.sequence_id << "|"
                    << event.event_id << "|"
                    << event.aggregate_type << "|"
                    << event.aggregate_id << "|"
                    << event.event_type << "|"
                    << event.payload_data << "|"
                    << event.valid_time << "|"
                    << event.transaction_time << "|"
                    << event.recorded_by << "|"
                    << event.previous_hash;

    event.event_hash = calculate_hash(payload_builder.str());
    latest_hash_ = event.event_hash;

    ledger_.push_back(std::move(event));
    return ledger_.back().sequence_id;
}

std::vector<BitemporalEvent> BitemporalEventStore::get_events_for_aggregate(
    const std::string& aggregate_type,
    const std::string& aggregate_id
) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    std::vector<BitemporalEvent> result;

    for (const auto& ev : ledger_) {
        if (ev.aggregate_type == aggregate_type && ev.aggregate_id == aggregate_id) {
            result.push_back(ev);
        }
    }
    return result;
}

std::vector<BitemporalEvent> BitemporalEventStore::query_transaction_as_of(
    const std::string& max_transaction_time
) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    std::vector<BitemporalEvent> result;

    for (const auto& ev : ledger_) {
        if (ev.transaction_time <= max_transaction_time) {
            result.push_back(ev);
        }
    }
    return result;
}

std::vector<BitemporalEvent> BitemporalEventStore::query_valid_as_of(
    const std::string& max_valid_time
) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    std::vector<BitemporalEvent> result;

    for (const auto& ev : ledger_) {
        if (ev.valid_time <= max_valid_time) {
            result.push_back(ev);
        }
    }
    return result;
}

bool BitemporalEventStore::verify_ledger_integrity() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    std::string expected_prev_hash = "GENESIS_HASH_DHARTI_CONTROL_PLANE_0000";

    for (size_t i = 0; i < ledger_.size(); ++i) {
        const auto& ev = ledger_[i];
        if (ev.sequence_id != (i + 1)) {
            return false; // Sequence broken
        }
        if (ev.previous_hash != expected_prev_hash) {
            return false; // Chain link broken
        }

        std::stringstream payload_builder;
        payload_builder << ev.sequence_id << "|"
                        << ev.event_id << "|"
                        << ev.aggregate_type << "|"
                        << ev.aggregate_id << "|"
                        << ev.event_type << "|"
                        << ev.payload_data << "|"
                        << ev.valid_time << "|"
                        << ev.transaction_time << "|"
                        << ev.recorded_by << "|"
                        << ev.previous_hash;

        std::string recalculated = calculate_hash(payload_builder.str());
        if (recalculated != ev.event_hash) {
            return false; // Content modified
        }

        expected_prev_hash = ev.event_hash;
    }

    return true;
}

size_t BitemporalEventStore::event_count() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return ledger_.size();
}

} // namespace core
} // namespace dharti
