#ifndef DHARTI_ADAPTERS_COURT_ADAPTER_HPP
#define DHARTI_ADAPTERS_COURT_ADAPTER_HPP

#include "dharti/adapters/adapter_interface.hpp"
#include <string>
#include <vector>
#include <optional>

namespace dharti {
namespace adapters {

/**
 * @brief Raw Court case docket ingested from eCourts CIS or State RCCMS.
 */
struct RawCourtDocket {
    std::string court_forum;       // e.g. "High Court", "Civil Court", "Revenue Court"
    std::string case_type;        // e.g. "Writ Petition", "Original Suit", "Title Dispute"
    std::string case_number;
    int32_t case_year{0};
    std::string petitioner;
    std::string respondent;
    std::string target_khasra_code;
    bool is_stay_granted{false};
    std::string stay_nature;      // "PossessionInjunction", "DisbursementStay", "StatusQuo"
    std::string order_date;
    bool is_stay_vacated{false};
    std::string vacate_date;
};

/**
 * @brief Normalized judicial encumbrance record mapped to a parcel.
 */
struct NormalizedCourtRecord {
    std::string case_urn;         // Unique eCourts Reference Number (CNR)
    std::string court_forum;
    std::string target_khasra_code;
    bool has_active_stay{false};
    std::string stay_category;
    std::string order_date;
    IngestionMetadata metadata;
};

/**
 * @brief Federated Adapter for Judicial Case Ingestion (eCourts CIS & State RCCMS).
 */
class CourtAdapter : public IAdapter {
public:
    explicit CourtAdapter(std::string court_jurisdiction = "NATIONAL_ECOURTS");
    ~CourtAdapter() override = default;

    std::string adapter_name() const override;
    std::string source_system_code() const override;
    std::string supported_schema_version() const override;

    /**
     * @brief Ingest and normalize an eCourts/RCCMS docket.
     * 
     * @param docket Raw case details.
     * @return NormalizedCourtRecord with active stay determination and provenance hash.
     */
    NormalizedCourtRecord ingest_docket(const RawCourtDocket& docket);

    /**
     * @brief Ingest eCourts / RCCMS dockets from real JSON file.
     */
    std::vector<NormalizedCourtRecord> ingest_from_file(const std::string& court_file_path);

    /**
     * @brief Check if a normalized record carries an active stay order.
     */
    bool is_stay_active(const NormalizedCourtRecord& record) const;

    size_t active_stays_tracked() const { return active_stay_count_; }

private:
    std::string jurisdiction_;
    size_t active_stay_count_{0};
};

} // namespace adapters
} // namespace dharti

#endif // DHARTI_ADAPTERS_COURT_ADAPTER_HPP
