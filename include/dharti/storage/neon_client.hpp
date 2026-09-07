#ifndef DHARTI_STORAGE_NEON_CLIENT_HPP
#define DHARTI_STORAGE_NEON_CLIENT_HPP

#include <string>
#include <vector>
#include <memory>
#include "dharti/models/evidence.hpp"

namespace dharti {
namespace storage {

/**
 * @brief Neon Serverless PostgreSQL Control-Plane Database Client in Pure C++.
 * Manages 4 layers: Sources/Health, Source Records, Snapshots, Artifacts & Events.
 */
class NeonClient {
public:
    explicit NeonClient(std::string db_url = "");

    /**
     * @brief Records a stable source record identifier and returns its UUID.
     */
    std::string upsert_source_record(
        const std::string& source_code,
        const std::string& source_record_id,
        const std::string& record_type
    );

    /**
     * @brief Inserts an immutable source snapshot record with its Google Drive pointer.
     */
    bool record_snapshot(const models::SourceSnapshot& snapshot);

    /**
     * @brief Inserts or updates an evidence artifact with SHA-256 and acceptance status.
     */
    bool record_evidence_artifact(const models::EvidenceArtifact& artifact);

    /**
     * @brief Appends a canonical workflow event to the immutable bitemporal ledger.
     */
    bool record_workflow_event(const models::CanonicalEvent& event);

    /**
     * @brief Records a quarantined incident or contradiction exception.
     */
    bool record_exception(const models::EvidenceException& ex);

    /**
     * @brief Updates source health metrics (lag, last attempt/success, status).
     */
    bool update_source_health(
        const std::string& source_code,
        const std::string& status,
        int64_t lag_seconds,
        bool success
    );

    /**
     * @brief Retrieves source health records for all sources.
     */
    std::vector<models::SourceHealth> get_all_source_health();

private:
    std::string m_db_url;

    /**
     * @brief Executes an SQL statement with parameter escaping via the secure DB transport.
     */
    bool execute_sql(const std::string& sql);
};

} // namespace storage
} // namespace dharti

#endif // DHARTI_STORAGE_NEON_CLIENT_HPP
