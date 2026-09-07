#ifndef DHARTI_SERVICES_POLLING_DAEMON_HPP
#define DHARTI_SERVICES_POLLING_DAEMON_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <atomic>

#include "dharti/scrapers/web_scraper.hpp"
#include "dharti/adapters/parivesh_adapter.hpp"
#include "dharti/adapters/bhoomi_rashi_adapter.hpp"
#include "dharti/services/evidence_engine.hpp"
#include "dharti/storage/gdrive_client.hpp"
#include "dharti/storage/neon_client.hpp"

namespace dharti {
namespace services {

/**
 * @brief Polling execution metrics and counters.
 */
struct PollingStats {
    uint64_t total_cycles{0};
    uint64_t total_sources_polled{0};
    uint64_t total_snapshots_archived{0};
    uint64_t total_events_emitted{0};
    uint64_t total_quarantined_contradictions{0};
    std::string last_poll_utc;
};

/**
 * @brief Specification for a National Priority Infrastructure Project.
 */
struct NationalProjectSpec {
    std::string project_id;
    std::string project_name;
    std::string highway_no;
    std::string clearance_proposal_no;
    std::string clearance_type;
    std::string clearance_stage;
    std::string gazette_no;
    std::string gazette_type;
    std::string state_code;
    std::string district;
    std::string taluk;
    double diversion_area_ha{0.0};
    double acquired_area_ha{0.0};
    std::string status;
    std::vector<std::string> survey_numbers;
};

/**
 * @brief Real-Time Pure C++ Automatic Polling Daemon.
 * 
 * Periodically polls official government portals (PARIVESH, Bhoomi Rashi, UP Bhulekh),
 * captures authentic HTTP status and raw payloads, computes RFC 6234 SHA-256 hashes,
 * compares against previous snapshots, evaluates 7-Rule Evidence Gate, archives into
 * structured Google Drive hierarchy ('dharti/raw/...'), and records canonical events
 * or quarantined contradictions in Neon Serverless PostgreSQL.
 */
class PollingDaemon {
public:
    explicit PollingDaemon(
        std::chrono::seconds poll_interval = std::chrono::hours(1),
        std::string vault_root = "dharti"
    );

    /**
     * @brief Executes a single polling iteration across baseline sources.
     */
    bool poll_once();

    /**
     * @brief Polls and archives real-world data for all major National Priority Infrastructure Corridors.
     */
    bool poll_national_projects();

    /**
     * @brief Polls a specific project or user-entered proposal/gazette dynamically.
     */
    bool poll_custom_project(const NationalProjectSpec& spec);

    /**
     * @brief Starts the continuous polling loop. Blocks until stop() is called or terminated.
     */
    void start();

    /**
     * @brief Gracefully terminates the running polling daemon.
     */
    void stop();

    bool is_running() const { return m_running; }
    PollingStats get_stats() const { return m_stats; }
    void set_interval(std::chrono::seconds interval) { m_interval = interval; }

    /**
     * @brief Injects a synthetic anomaly for live circuit breaker & quarantine demonstration.
     */
    void inject_simulated_anomaly(bool enable) { m_simulate_anomaly = enable; }

private:
    void poll_parivesh();
    void poll_bhoomirashi();
    void poll_upbhulekh();

    std::chrono::seconds m_interval;
    std::string m_vault_root;
    std::atomic<bool> m_running{false};
    bool m_simulate_anomaly{false};
    PollingStats m_stats;

    scrapers::WebScraper m_scraper;
    adapters::PariveshAdapter m_parivesh_adapter;
    adapters::BhoomiRashiAdapter m_bhoomi_adapter;
    services::EvidenceEngine m_evidence_engine;
    storage::GDriveClient m_gdrive;
    storage::NeonClient m_neon;

    // In-memory previous snapshot registry
    std::unordered_map<std::string, adapters::NormalizedClearanceRecord> m_last_clearance_snapshots;
    std::unordered_map<std::string, adapters::NormalizedGazetteRecord> m_last_gazette_snapshots;
    std::unordered_map<std::string, std::string> m_last_drive_ids;
};

} // namespace services
} // namespace dharti

#endif // DHARTI_SERVICES_POLLING_DAEMON_HPP
