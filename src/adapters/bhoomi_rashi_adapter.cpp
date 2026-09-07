#include "dharti/adapters/bhoomi_rashi_adapter.hpp"
#include "dharti/utils/sha256.hpp"

#include <sstream>
#include <iomanip>
#include <chrono>

namespace dharti {
namespace adapters {

BhoomiRashiAdapter::BhoomiRashiAdapter(std::string region)
    : m_region(std::move(region)) {}

NormalizedGazetteRecord BhoomiRashiAdapter::normalize(const RawGazetteNotification& raw) {
    NormalizedGazetteRecord rec;
    rec.notification_number = raw.notification_number;
    rec.gazette_type = raw.gazette_type;
    rec.highway_number = raw.highway_number;
    rec.project_name = raw.project_name;
    rec.state_code = raw.state_code;
    rec.district = raw.district;
    rec.acquired_area_ha = raw.acquired_area_ha;
    rec.published_date = raw.published_date;

    std::stringstream ss;
    ss << "{\n"
       << "  \"source\": \"BHOOMI_RASHI\",\n"
       << "  \"notification_number\": \"" << raw.notification_number << "\",\n"
       << "  \"gazette_type\": \"" << raw.gazette_type << "\",\n"
       << "  \"highway_number\": \"" << raw.highway_number << "\",\n"
       << "  \"project_name\": \"" << raw.project_name << "\",\n"
       << "  \"state_code\": \"" << raw.state_code << "\",\n"
       << "  \"district\": \"" << raw.district << "\",\n"
       << "  \"taluk\": \"" << raw.taluk << "\",\n"
       << "  \"acquired_area_ha\": " << std::fixed << std::setprecision(4) << raw.acquired_area_ha << ",\n"
       << "  \"published_date\": \"" << raw.published_date << "\",\n"
       << "  \"survey_numbers\": [";

    for (size_t i = 0; i < raw.survey_numbers.size(); ++i) {
        ss << "\"" << raw.survey_numbers[i] << "\"";
        if (i + 1 < raw.survey_numbers.size()) ss << ", ";
    }
    ss << "]\n}";

    rec.json_payload = ss.str();

    // Provenance Metadata
    rec.metadata.source_system = "BHOOMI_RASHI";
    rec.metadata.source_record_id = raw.notification_number;
    rec.metadata.schema_version = "nhai-gazette-1956-v1";
    rec.metadata.payload_checksum = utils::SHA256::hash_string(rec.json_payload);

    auto now = std::chrono::system_clock::now();
    auto in_time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ts;
    ts << std::put_time(std::gmtime(&in_time), "%Y-%m-%dT%H:%M:%SZ");
    rec.metadata.ingestion_timestamp = ts.str();

    return rec;
}

} // namespace adapters
} // namespace dharti
