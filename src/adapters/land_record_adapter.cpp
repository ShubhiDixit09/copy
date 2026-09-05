#include "dharti/adapters/land_record_adapter.hpp"
#include "dharti/utils/json.hpp"
#include <sstream>
#include <map>

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

std::vector<NormalizedLandRecord> LandRecordAdapter::ingest_from_files(
    const std::string& ror_file_path,
    const std::string& cadastral_file_path
) {
    std::vector<NormalizedLandRecord> results;

    utils::JsonValue ror_json = utils::JsonValue::parse_file(ror_file_path);
    utils::JsonValue cad_json = utils::JsonValue::parse_file(cadastral_file_path);

    // Index RoR records by survey_number + "/" + sub_division
    std::map<std::string, RawRoRRecord> ror_map;
    const auto& ror_records = ror_json["records"];
    for (size_t i = 0; i < ror_records.size(); ++i) {
        const auto& r = ror_records[i];
        RawRoRRecord rec;
        rec.state_code = ror_json["state_code"].as_string("KA");
        rec.district = ror_json["district"].as_string();
        rec.taluk = ror_json["taluk"].as_string();
        rec.village = ror_json["village"].as_string();
        rec.survey_number = r["survey_number"].as_string();
        rec.sub_division = r["sub_division"].as_string();
        rec.owner_name = r["owner_name"].as_string();
        rec.ror_area_sqm = r["extent_sqm"].as_double();
        rec.tenure_type = r["tenure_type"].as_string();
        rec.has_encumbrance = r["is_encumbered"].as_bool();
        rec.encumbrance_details = r["encumbrance_details"].as_string();

        std::string key = rec.survey_number + "/" + rec.sub_division;
        ror_map[key] = rec;
    }

    // Process Cadastral parcels
    const auto& cad_parcels = cad_json["parcels"];
    for (size_t i = 0; i < cad_parcels.size(); ++i) {
        const auto& p = cad_parcels[i];
        RawCadastralRecord cad;
        cad.survey_number = p["survey_number"].as_string();
        cad.sub_division = p["sub_division"].as_string();
        cad.polygon_area_sqm = p["polygon_area_sqm"].as_double();
        cad.chainage_start_km = p["chainage_start_km"].as_double();
        cad.chainage_end_km = p["chainage_end_km"].as_double();
        cad.crs_projection = cad_json["crs"].as_string("EPSG:4326");
        cad.boundary_wkt = p["boundary_wkt"].as_string();

        int64_t parcel_id = p["parcel_id"].as_int(i + 1);
        std::string key = cad.survey_number + "/" + cad.sub_division;

        RawRoRRecord ror;
        if (ror_map.find(key) != ror_map.end()) {
            ror = ror_map[key];
        } else {
            ror.survey_number = cad.survey_number;
            ror.sub_division = cad.sub_division;
        }

        results.push_back(ingest_and_harmonize(ror, cad, parcel_id));
    }

    return results;
}

} // namespace adapters
} // namespace dharti

