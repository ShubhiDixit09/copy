#ifndef DHARTI_ADAPTERS_BHOOMI_RASHI_ADAPTER_HPP
#define DHARTI_ADAPTERS_BHOOMI_RASHI_ADAPTER_HPP

#include <string>
#include <vector>
#include "dharti/adapters/adapter_interface.hpp"

namespace dharti {
namespace adapters {

/**
 * @brief Raw Land Acquisition Gazette Notification from MoRTH Bhoomi Rashi Portal.
 * Corresponds to statutory notifications under the National Highways Act, 1956.
 */
struct RawGazetteNotification {
    std::string notification_number; // e.g. "S.O. 3842(E)"
    std::string gazette_type;        // "3a", "3A", "3D", "3G"
    std::string highway_number;      // e.g. "NH-48"
    std::string project_name;        // e.g. "Bengaluru-Chennai Expressway Package-2"
    std::string state_code;          // e.g. "KA"
    std::string district;            // e.g. "Bengaluru Rural"
    std::string taluk;               // e.g. "Hosakote"
    double acquired_area_ha{0.0};    // Area in Hectares
    std::string published_date;      // e.g. "2026-08-15"
    std::vector<std::string> survey_numbers;
};

/**
 * @brief Normalized Bhoomi Rashi Gazette record ready for cryptographic hashing and Neon DB.
 */
struct NormalizedGazetteRecord {
    std::string notification_number;
    std::string gazette_type;
    std::string highway_number;
    std::string project_name;
    std::string state_code;
    std::string district;
    double acquired_area_ha{0.0};
    std::string published_date;
    std::string json_payload;
    IngestionMetadata metadata;
};

/**
 * @brief Adapter for parsing and normalizing MoRTH Bhoomi Rashi Land Acquisition notifications.
 */
class BhoomiRashiAdapter {
public:
    explicit BhoomiRashiAdapter(std::string region = "ALL_INDIA");

    NormalizedGazetteRecord normalize(const RawGazetteNotification& raw);

private:
    std::string m_region;
};

} // namespace adapters
} // namespace dharti

#endif // DHARTI_ADAPTERS_BHOOMI_RASHI_ADAPTER_HPP
