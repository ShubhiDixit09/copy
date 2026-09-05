#ifndef DHARTI_ADAPTERS_LAND_RECORD_ADAPTER_HPP
#define DHARTI_ADAPTERS_LAND_RECORD_ADAPTER_HPP

#include "dharti/adapters/adapter_interface.hpp"
#include "dharti/models/parcel.hpp"
#include <string>
#include <vector>
#include <optional>

namespace dharti {
namespace adapters {

/**
 * @brief Raw Record of Rights (RoR) data structure ingested from State revenue portals.
 */
struct RawRoRRecord {
    std::string state_code;        // e.g. "KA", "MH", "UP"
    std::string district;
    std::string taluk;
    std::string village;
    std::string survey_number;
    std::string sub_division;
    std::string owner_name;
    double ror_area_sqm{0.0};
    std::string tenure_type;       // e.g. "Ryotwari", "Inam", "Government"
    bool has_encumbrance{false};
    std::string encumbrance_details;
};

/**
 * @brief Raw Cadastral GIS record containing surveyed polygon coordinates and chainage.
 */
struct RawCadastralRecord {
    std::string survey_number;
    std::string sub_division;
    double polygon_area_sqm{0.0};
    double chainage_start_km{0.0};
    double chainage_end_km{0.0};
    std::string crs_projection;    // e.g. "EPSG:4326", "EPSG:3857"
    std::string boundary_wkt;
};

/**
 * @brief Normalized land parcel snapshot ready for domain processing.
 */
struct NormalizedLandRecord {
    int64_t parcel_id{0};
    std::string canonical_khasra_code;
    std::string owner_name;
    double ror_area_sqm{0.0};
    double cadastral_area_sqm{0.0};
    double chainage_start_km{0.0};
    double chainage_end_km{0.0};
    bool has_active_encumbrance{false};
    IngestionMetadata metadata;
};

/**
 * @brief Federated Adapter for State Land Records and Cadastral Vector Ingestion.
 */
class LandRecordAdapter : public IAdapter {
public:
    explicit LandRecordAdapter(std::string state_code = "GENERIC_STATE");
    ~LandRecordAdapter() override = default;

    std::string adapter_name() const override;
    std::string source_system_code() const override;
    std::string supported_schema_version() const override;

    /**
     * @brief Ingest and harmonize an RoR record with cadastral survey geometry.
     * 
     * @param ror Raw Record of Rights from State portal.
     * @param cadastral Raw spatial cadastral polygon survey.
     * @param parcel_id Assigned internal system parcel sequence ID.
     * @return NormalizedLandRecord containing verified fields or quarantine flags.
     */
    NormalizedLandRecord ingest_and_harmonize(
        const RawRoRRecord& ror,
        const RawCadastralRecord& cadastral,
        int64_t parcel_id
    );

    /**
     * @brief Retrieve count of successfully ingested records.
     */
    size_t successful_ingestions() const { return success_count_; }

    /**
     * @brief Retrieve count of quarantined records.
     */
    size_t quarantined_records() const { return quarantine_count_; }

private:
    std::string state_code_;
    size_t success_count_{0};
    size_t quarantine_count_{0};
};

} // namespace adapters
} // namespace dharti

#endif // DHARTI_ADAPTERS_LAND_RECORD_ADAPTER_HPP
