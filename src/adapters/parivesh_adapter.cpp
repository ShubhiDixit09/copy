#include "dharti/adapters/parivesh_adapter.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace dharti {
namespace adapters {

PariveshAdapter::PariveshAdapter(std::string region_code)
    : m_region_code(std::move(region_code)) {}

std::string PariveshAdapter::serialize_to_json(const RawClearanceProposal& raw) {
    std::stringstream ss;
    ss << "{\n"
       << "  \"proposal_no\": \"" << raw.proposal_no << "\",\n"
       << "  \"project_name\": \"" << raw.project_name << "\",\n"
       << "  \"clearance_category\": \"" << raw.clearance_category << "\",\n"
       << "  \"stage\": \"" << raw.stage << "\",\n"
       << "  \"state_code\": \"" << raw.state_code << "\",\n"
       << "  \"district\": \"" << raw.district << "\",\n"
       << "  \"diversion_area_ha\": " << std::fixed << std::setprecision(2) << raw.diversion_area_ha << ",\n"
       << "  \"trees_to_fell\": " << raw.trees_to_fell << ",\n"
       << "  \"current_status\": \"" << raw.current_status << "\",\n"
       << "  \"submission_date\": \"" << raw.submission_date << "\",\n"
       << "  \"decision_date\": \"" << raw.decision_date << "\",\n"
       << "  \"issuing_authority\": \"" << raw.issuing_authority << "\",\n"
       << "  \"conditions\": [";
    for (size_t i = 0; i < raw.conditions.size(); ++i) {
        ss << "\"" << raw.conditions[i] << "\"";
        if (i + 1 < raw.conditions.size()) ss << ", ";
    }
    ss << "]\n}";
    return ss.str();
}

NormalizedClearanceRecord PariveshAdapter::normalize(const RawClearanceProposal& raw) {
    NormalizedClearanceRecord rec;
    rec.proposal_no = raw.proposal_no;
    rec.clearance_type = raw.clearance_category;
    rec.stage = raw.stage;
    rec.state_code = raw.state_code;
    rec.district = raw.district;
    rec.diversion_area_ha = raw.diversion_area_ha;
    rec.trees_to_fell = raw.trees_to_fell;
    rec.decision_date = raw.decision_date;

    // Standardize status string
    std::string s = raw.current_status;
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    if (s.find("APPROV") != std::string::npos) {
        rec.status = "APPROVED";
        rec.is_approved = true;
        m_approvals_tracked++;
    } else if (s.find("REJECT") != std::string::npos) {
        rec.status = "REJECTED";
        rec.is_approved = false;
    } else {
        rec.status = "UNDER_PROCESS";
        rec.is_approved = false;
    }

    rec.json_payload = serialize_to_json(raw);

    // Compute cryptographic SHA-256
    std::string sha256 = utils::SHA256::hash_string(rec.json_payload);

    // Set provenance metadata
    rec.metadata.source_system = source_system_code();
    rec.metadata.source_record_id = raw.proposal_no;
    rec.metadata.schema_version = supported_schema_version();
    rec.metadata.payload_checksum = sha256;
    rec.metadata.ingestion_timestamp = current_utc_timestamp();
    rec.metadata.is_quarantined = false;

    m_total_processed++;
    return rec;
}

} // namespace adapters
} // namespace dharti
