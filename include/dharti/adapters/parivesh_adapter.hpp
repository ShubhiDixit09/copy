#ifndef DHARTI_ADAPTERS_PARIVESH_ADAPTER_HPP
#define DHARTI_ADAPTERS_PARIVESH_ADAPTER_HPP

#include <string>
#include <vector>
#include "dharti/adapters/adapter_interface.hpp"
#include "dharti/utils/sha256.hpp"

namespace dharti {
namespace adapters {

/**
 * @brief Raw proposal record ingested from MoEFCC PARIVESH single-window portal.
 */
struct RawClearanceProposal {
    std::string proposal_no;          // e.g. "IA/KA/NHA/10482/2026"
    std::string project_name;         // "Bengaluru-Chennai Expressway Package 2"
    std::string clearance_category;   // "FOREST_CLEARANCE", "ENVIRONMENT_CLEARANCE", "WILDLIFE"
    std::string stage;                // "STAGE_1", "STAGE_2", "FINAL"
    std::string state_code;           // "KA"
    std::string district;             // "Bengaluru Rural"
    double diversion_area_ha{0.0};    // Forest land diversion area in Hectares
    int trees_to_fell{0};
    std::string current_status;       // "Under Process", "Approved", "Rejected"
    std::string submission_date;      // ISO 8601 YYYY-MM-DD
    std::string decision_date;        // ISO 8601 YYYY-MM-DD
    std::vector<std::string> conditions; // Statutory afforestation/mitigation conditions
    std::string issuing_authority{"MoEFCC Regional Office Bengaluru"};
};

/**
 * @brief Canonical normalized representation of a statutory clearance.
 */
struct NormalizedClearanceRecord {
    IngestionMetadata metadata;
    std::string proposal_no;
    std::string clearance_type;
    std::string stage;
    std::string state_code;
    std::string district;
    double diversion_area_ha{0.0};
    int trees_to_fell{0};
    std::string status;               // "UNDER_PROCESS", "APPROVED", "REJECTED"
    bool is_approved{false};
    std::string decision_date;
    std::string json_payload;         // Normalized JSON representation
};

/**
 * @brief MoEFCC PARIVESH single-window clearance adapter.
 */
class PariveshAdapter : public IAdapter {
public:
    explicit PariveshAdapter(std::string region_code = "SZ_REGIONAL");
    ~PariveshAdapter() override = default;

    std::string adapter_name() const override { return "PariveshMoEFCCAdapter"; }
    std::string source_system_code() const override { return "PARIVESH"; }
    std::string supported_schema_version() const override { return "v2.1.0"; }

    /**
     * @brief Normalizes raw proposal record into canonical clearance record.
     * Computes RFC 6234 SHA-256 checksum over the serialized payload.
     */
    NormalizedClearanceRecord normalize(const RawClearanceProposal& raw);

    /**
     * @brief Serializes normalized clearance to JSON string.
     */
    static std::string serialize_to_json(const RawClearanceProposal& raw);

    uint64_t total_processed() const { return m_total_processed; }
    uint64_t approvals_tracked() const { return m_approvals_tracked; }

private:
    std::string m_region_code;
    uint64_t m_total_processed{0};
    uint64_t m_approvals_tracked{0};
};

} // namespace adapters
} // namespace dharti

#endif // DHARTI_ADAPTERS_PARIVESH_ADAPTER_HPP
