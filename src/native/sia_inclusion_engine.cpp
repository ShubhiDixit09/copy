#include "dharti/services/sia_inclusion_engine.hpp"
#include "dharti/utils/json.hpp"

namespace dharti {
namespace services {

core::ServiceHealth SIAInclusionEngine::health_check() const {
    return core::ServiceHealth{
        service_name(),
        core::HealthStatus::HEALTHY,
        "SIA Inclusion Safeguard Engine operational",
        1757088000000ULL
    };
}

std::string SIAInclusionEngine::service_name() const {
    return "SIAInclusionEngine";
}

SIAAuditReport SIAInclusionEngine::audit_universe(
    const std::vector<models::Household>& universe
) const {
    SIAAuditReport report;
    report.total_households_observed = static_cast<int>(universe.size());
    report.award_mapped_count = 0;
    report.rnr_mapped_count = 0;
    report.lawfully_excluded_count = 0;
    report.missing_unaccounted_count = 0;

    for (const auto& hh : universe) {
        if (hh.has_award_mapping) {
            report.award_mapped_count++;
        }
        if (hh.has_rnr_mapping) {
            report.rnr_mapped_count++;
        }
        if (hh.is_reviewed_ineligible && !hh.speaking_order_ref.empty()) {
            report.lawfully_excluded_count++;
        }

        // An affected household MUST be in award, in R&R, or have a formal speaking order documenting ineligibility.
        // If none of these exist, it is an illegal exclusion violation.
        bool is_accounted = hh.has_award_mapping || hh.has_rnr_mapping ||
                            (hh.is_reviewed_ineligible && !hh.speaking_order_ref.empty());

        if (!is_accounted) {
            report.missing_unaccounted_count++;
            report.missing_households.push_back(hh);
        }
    }

    report.is_compliant = (report.missing_unaccounted_count == 0);
    return report;
}

std::vector<models::ContradictionCase> SIAInclusionEngine::generate_exception_cases(
    const std::vector<models::Household>& missing_households
) const {
    std::vector<models::ContradictionCase> cases;
    for (const auto& hh : missing_households) {
        models::ContradictionCase c;
        c.case_id = "EX-SIA-MISSING-" + hh.household_id;
        c.parcel_id = 0; // Cross-cutting household exception
        c.type = models::ContradictionType::UNMATCHED_SIA_HOUSEHOLD;
        c.severity = models::ContradictionSeverity::CRITICAL;
        c.description = "Household " + hh.household_id + " (" + hh.category +
                        ", " + std::to_string(hh.family_member_count) + " members, vulnerable=" +
                        (hh.is_vulnerable ? "YES" : "NO") + ") present in SIA survey universe but missing from Award & R&R schedules.";
        c.blocking_gate = "AwardGate";
        c.assigned_owner_role = "RROfficer";
        c.sla_hours = 48;
        c.is_resolved = false;
        cases.push_back(c);
    }
    return cases;
}

std::vector<models::Household> SIAInclusionEngine::ingest_from_file(const std::string& sia_file_path) const {
    std::vector<models::Household> universe;
    utils::JsonValue root = utils::JsonValue::parse_file(sia_file_path);

    const auto& households = root["households"];
    for (size_t i = 0; i < households.size(); ++i) {
        const auto& h = households[i];
        models::Household hh;
        hh.household_id = h["household_id"].as_string();
        hh.category = h["category"].as_string();
        hh.family_member_count = static_cast<int>(h["family_member_count"].as_int(1));
        hh.is_vulnerable = h["is_vulnerable"].as_bool();
        hh.has_award_mapping = h["has_award_mapping"].as_bool();
        hh.has_rnr_mapping = h["has_rnr_mapping"].as_bool();
        hh.is_reviewed_ineligible = h["is_reviewed_ineligible"].as_bool();

        universe.push_back(hh);
    }

    return universe;
}

} // namespace services
} // namespace dharti

