#include "dharti_c_api.h"
#include "dharti/services/contradiction_engine.hpp"
#include "dharti/services/sia_inclusion_engine.hpp"
#include <cstring>
#include <algorithm>

extern "C" {

DHARTI_EXPORT int evaluate_parcel_contradictions_native(
    const CParcelInput* parcel,
    double area_tolerance_percent,
    CContradictionCase* out_cases,
    int max_cases,
    int* actual_cases_count
) {
    if (!parcel || !out_cases || !actual_cases_count) {
        return -1;
    }

    dharti::models::ParcelVersion pv;
    pv.parcel_id = parcel->parcel_id;
    pv.ror_area_sqm = parcel->ror_area_sqm;
    pv.cadastral_area_sqm = parcel->cadastral_area_sqm;
    pv.ror_owner = parcel->ror_owner ? parcel->ror_owner : "";
    pv.field_claimant = parcel->field_claimant ? parcel->field_claimant : "";
    pv.has_active_stay = (parcel->has_active_stay != 0);

    dharti::services::ContradictionEngine engine(area_tolerance_percent);
    auto cases = engine.evaluate_parcel(pv);

    int copy_count = std::min(static_cast<int>(cases.size()), max_cases);
    for (int i = 0; i < copy_count; ++i) {
        out_cases[i].parcel_id = cases[i].parcel_id;
        out_cases[i].type_code = static_cast<int>(cases[i].type);
        out_cases[i].severity_code = static_cast<int>(cases[i].severity);
        out_cases[i].sla_hours = cases[i].sla_hours;

        std::strncpy(out_cases[i].case_id, cases[i].case_id.c_str(), sizeof(out_cases[i].case_id) - 1);
        out_cases[i].case_id[sizeof(out_cases[i].case_id) - 1] = '\0';

        std::strncpy(out_cases[i].description, cases[i].description.c_str(), sizeof(out_cases[i].description) - 1);
        out_cases[i].description[sizeof(out_cases[i].description) - 1] = '\0';

        std::strncpy(out_cases[i].blocking_gate, cases[i].blocking_gate.c_str(), sizeof(out_cases[i].blocking_gate) - 1);
        out_cases[i].blocking_gate[sizeof(out_cases[i].blocking_gate) - 1] = '\0';

        std::strncpy(out_cases[i].assigned_owner, cases[i].assigned_owner_role.c_str(), sizeof(out_cases[i].assigned_owner) - 1);
        out_cases[i].assigned_owner[sizeof(out_cases[i].assigned_owner) - 1] = '\0';
    }

    *actual_cases_count = copy_count;
    return 0;
}

DHARTI_EXPORT int audit_sia_inclusion_native(
    const CHouseholdInput* households,
    int count,
    CSIAAuditSummary* out_summary
) {
    if (!households || !out_summary) {
        return -1;
    }

    std::vector<dharti::models::Household> universe;
    universe.reserve(count);

    for (int i = 0; i < count; ++i) {
        dharti::models::Household h;
        h.household_id = households[i].household_id ? households[i].household_id : "";
        h.category = households[i].category ? households[i].category : "";
        h.family_member_count = households[i].family_member_count;
        h.is_vulnerable = (households[i].is_vulnerable != 0);
        h.has_award_mapping = (households[i].has_award_mapping != 0);
        h.has_rnr_mapping = (households[i].has_rnr_mapping != 0);
        h.is_reviewed_ineligible = (households[i].is_reviewed_ineligible != 0);
        h.speaking_order_ref = households[i].speaking_order_ref ? households[i].speaking_order_ref : "";
        universe.push_back(h);
    }

    dharti::services::SIAInclusionEngine engine;
    auto report = engine.audit_universe(universe);

    out_summary->total_observed = report.total_households_observed;
    out_summary->award_mapped = report.award_mapped_count;
    out_summary->rnr_mapped = report.rnr_mapped_count;
    out_summary->lawfully_excluded = report.lawfully_excluded_count;
    out_summary->missing_unaccounted = report.missing_unaccounted_count;
    out_summary->is_compliant = report.is_compliant ? 1 : 0;

    return 0;
}

} // extern "C"
