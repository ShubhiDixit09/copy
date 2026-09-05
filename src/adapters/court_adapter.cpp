#include "dharti/adapters/court_adapter.hpp"
#include "dharti/utils/json.hpp"
#include <sstream>

namespace dharti {
namespace adapters {

CourtAdapter::CourtAdapter(std::string court_jurisdiction)
    : jurisdiction_(std::move(court_jurisdiction)) {}

std::string CourtAdapter::adapter_name() const {
    return "JudicialFederatedAdapter-" + jurisdiction_;
}

std::string CourtAdapter::source_system_code() const {
    return "ECOURTS_" + jurisdiction_;
}

std::string CourtAdapter::supported_schema_version() const {
    return "v1.4-CNR-CaseDocket";
}

NormalizedCourtRecord CourtAdapter::ingest_docket(const RawCourtDocket& docket) {
    NormalizedCourtRecord record;
    record.court_forum = docket.court_forum;
    record.target_khasra_code = docket.target_khasra_code;
    record.case_urn = docket.court_forum + "/" + docket.case_type + "/" + docket.case_number + "/" + std::to_string(docket.case_year);
    
    // An active stay exists if stay was granted and has not yet been vacated
    record.has_active_stay = docket.is_stay_granted && !docket.is_stay_vacated;
    record.stay_category = docket.stay_nature;
    record.order_date = docket.is_stay_vacated ? docket.vacate_date : docket.order_date;

    std::stringstream raw_builder;
    raw_builder << record.case_urn << "|"
                << docket.target_khasra_code << "|"
                << docket.petitioner << "|"
                << docket.respondent << "|"
                << docket.is_stay_granted << "|"
                << docket.is_stay_vacated;

    record.metadata.source_system = source_system_code();
    record.metadata.source_record_id = record.case_urn;
    record.metadata.schema_version = supported_schema_version();
    record.metadata.payload_checksum = compute_hash(raw_builder.str());
    record.metadata.ingestion_timestamp = current_utc_timestamp();

    if (docket.case_number.empty() || docket.case_year <= 1900) {
        record.metadata.is_quarantined = true;
        record.metadata.quarantine_reason = "Invalid case number or historic year.";
    } else {
        record.metadata.is_quarantined = false;
        if (record.has_active_stay) {
            active_stay_count_++;
        }
    }

    return record;
}

bool CourtAdapter::is_stay_active(const NormalizedCourtRecord& record) const {
    return !record.metadata.is_quarantined && record.has_active_stay;
}

std::vector<NormalizedCourtRecord> CourtAdapter::ingest_from_file(const std::string& court_file_path) {
    std::vector<NormalizedCourtRecord> results;
    utils::JsonValue root = utils::JsonValue::parse_file(court_file_path);

    std::string default_forum = root["court_jurisdiction"].as_string("High Court of Karnataka");
    const auto& dockets = root["dockets"];

    for (size_t i = 0; i < dockets.size(); ++i) {
        const auto& d = dockets[i];
        RawCourtDocket raw;
        raw.court_forum = d["court_forum"].as_string(default_forum);
        raw.case_type = d["case_type"].as_string();
        raw.case_number = d["case_number"].as_string();
        raw.case_year = static_cast<int32_t>(d["case_year"].as_int(2023));
        raw.petitioner = d["petitioner"].as_string();
        raw.respondent = d["respondent"].as_string();
        raw.target_khasra_code = d["target_khasra_code"].as_string();
        raw.is_stay_granted = d["is_stay_granted"].as_bool();
        raw.stay_nature = d["stay_nature"].as_string();
        raw.order_date = d["order_date"].as_string();
        raw.is_stay_vacated = d["is_stay_vacated"].as_bool();
        raw.vacate_date = d["vacate_date"].as_string();

        results.push_back(ingest_docket(raw));
    }

    return results;
}

} // namespace adapters
} // namespace dharti

