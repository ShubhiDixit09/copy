#include "dharti/adapters/land_record_adapter.hpp"
#include <sstream>

namespace dharti {
namespace adapters {

LandRecordAdapter::LandRecordAdapter(std::string state_code)
    : state_code_(std::move(state_code)) {}

std::string LandRecordAdapter::adapter_name() const {
    return "LandRecordFederatedAdapter-" + state_code_;
}

std::string LandRecordAdapter::source_system_code() const {
    return "REV_DEPT_" + state_code_;
}

std::string LandRecordAdapter::supported_schema_version() const {
    return "v2.1-CadastralVector";
}

NormalizedLandRecord LandRecordAdapter::ingest_and_harmonize(
    const RawRoRRecord& ror,
    const RawCadastralRecord& cadastral,
    int64_t parcel_id
) {
    NormalizedLandRecord record;
    record.parcel_id = parcel_id;
    record.canonical_khasra_code = state_code_ + "/" + ror.district + "/" + ror.village + "/" + ror.survey_number;
    if (!ror.sub_division.empty()) {
        record.canonical_khasra_code += "/" + ror.sub_division;
    }

    record.owner_name = ror.owner_name;
    record.ror_area_sqm = ror.ror_area_sqm;
    record.cadastral_area_sqm = cadastral.polygon_area_sqm;
    record.chainage_start_km = cadastral.chainage_start_km;
    record.chainage_end_km = cadastral.chainage_end_km;
    record.has_active_encumbrance = ror.has_encumbrance;

    // Build metadata and compute cryptographic hash for provenance
    std::stringstream raw_builder;
    raw_builder << record.canonical_khasra_code << "|"
                << ror.owner_name << "|"
                << ror.ror_area_sqm << "|"
                << cadastral.polygon_area_sqm << "|"
                << cadastral.chainage_start_km << "|"
                << cadastral.chainage_end_km;

    record.metadata.source_system = source_system_code();
    record.metadata.source_record_id = ror.survey_number + "-" + ror.sub_division;
    record.metadata.schema_version = supported_schema_version();
    record.metadata.payload_checksum = compute_hash(raw_builder.str());
    record.metadata.ingestion_timestamp = current_utc_timestamp();

    // Data validation rules
    bool is_valid = true;
    std::string err_msg;

    if (ror.owner_name.empty()) {
        is_valid = false;
        err_msg = "Owner name is empty in RoR record.";
    } else if (ror.ror_area_sqm <= 0.0) {
        is_valid = false;
        err_msg = "RoR stated area must be strictly positive.";
    } else if (cadastral.polygon_area_sqm <= 0.0) {
        is_valid = false;
        err_msg = "Cadastral surveyed area must be strictly positive.";
    } else if (cadastral.chainage_start_km >= cadastral.chainage_end_km) {
        is_valid = false;
        err_msg = "Cadastral chainage start must be strictly less than end chainage.";
    }

    if (!is_valid) {
        record.metadata.is_quarantined = true;
        record.metadata.quarantine_reason = err_msg;
        quarantine_count_++;
    } else {
        record.metadata.is_quarantined = false;
        success_count_++;
    }

    return record;
}

} // namespace adapters
} // namespace dharti
