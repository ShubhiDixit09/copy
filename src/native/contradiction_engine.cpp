#include "dharti/services/contradiction_engine.hpp"
#include <cmath>
#include <sstream>

namespace dharti {
namespace services {

ContradictionEngine::ContradictionEngine(double area_tolerance_percent)
    : area_tolerance_percent_(area_tolerance_percent) {}

core::ServiceHealth ContradictionEngine::health_check() const {
    return core::ServiceHealth{
        service_name(),
        core::HealthStatus::HEALTHY,
        "Contradiction Engine online; rules active",
        1757088000000ULL
    };
}

std::string ContradictionEngine::service_name() const {
    return "ContradictionEngine";
}

std::vector<models::ContradictionCase> ContradictionEngine::evaluate_parcel(
    const models::ParcelVersion& parcel
) const {
    std::vector<models::ContradictionCase> exceptions;

    // Rule 1: Active Judicial/Revenue Court Injunction
    if (parcel.has_active_stay) {
        models::ContradictionCase c;
        c.case_id = "EX-STAY-" + std::to_string(parcel.parcel_id);
        c.parcel_id = parcel.parcel_id;
        c.type = models::ContradictionType::ACTIVE_COURT_STAY;
        c.severity = models::ContradictionSeverity::CRITICAL;
        c.description = "Active stay order / status quo injunction recorded in eCourts/RCCMS registry.";
        c.blocking_gate = "PossessionAndConstructionGate";
        c.assigned_owner_role = "LegalOfficer";
        c.sla_hours = 48;
        c.is_resolved = false;
        exceptions.push_back(c);
    }

    // Rule 2: Title and Possession Claimant Mismatch
    if (!parcel.ror_owner.empty() && !parcel.field_claimant.empty() &&
        parcel.ror_owner != parcel.field_claimant) {
        models::ContradictionCase c;
        c.case_id = "EX-TITLE-" + std::to_string(parcel.parcel_id);
        c.parcel_id = parcel.parcel_id;
        c.type = models::ContradictionType::TITLE_MISMATCH;
        c.severity = models::ContradictionSeverity::CRITICAL;
        c.description = "Registered RoR titleholder (" + parcel.ror_owner +
                        ") contradicts on-ground surveyed claimant (" + parcel.field_claimant + "). Succession or unregistered sale dispute.";
        c.blocking_gate = "AwardGate";
        c.assigned_owner_role = "CompetentAuthorityLAA";
        c.sla_hours = 72;
        c.is_resolved = false;
        exceptions.push_back(c);
    }

    // Rule 3: Cadastral Map vs RoR Area Discrepancy
    if (parcel.ror_area_sqm > 0.0 && parcel.cadastral_area_sqm > 0.0) {
        double diff = std::abs(parcel.cadastral_area_sqm - parcel.ror_area_sqm);
        double percent_diff = (diff / parcel.ror_area_sqm) * 100.0;
        if (percent_diff > area_tolerance_percent_) {
            models::ContradictionCase c;
            c.case_id = "EX-AREA-" + std::to_string(parcel.parcel_id);
            c.parcel_id = parcel.parcel_id;
            c.type = models::ContradictionType::AREA_DISCREPANCY;
            c.severity = (percent_diff > 5.0) ? models::ContradictionSeverity::CRITICAL : models::ContradictionSeverity::WARNING;
            std::ostringstream oss;
            oss << "Cadastral vector area (" << parcel.cadastral_area_sqm
                << " sqm) differs from recorded RoR area (" << parcel.ror_area_sqm
                << " sqm) by " << percent_diff << "% (threshold: " << area_tolerance_percent_ << "%).";
            c.description = oss.str();
            c.blocking_gate = (c.severity == models::ContradictionSeverity::CRITICAL) ? "AwardGate" : "NoticeGate";
            c.assigned_owner_role = "DistrictSurveyOfficer";
            c.sla_hours = 96;
            c.is_resolved = false;
            exceptions.push_back(c);
        }
    }

    return exceptions;
}

std::vector<models::ContradictionCase> ContradictionEngine::evaluate_portfolio(
    const std::vector<models::ParcelVersion>& parcels
) const {
    std::vector<models::ContradictionCase> all_exceptions;
    for (const auto& p : parcels) {
        auto parcel_exceptions = evaluate_parcel(p);
        all_exceptions.insert(all_exceptions.end(), parcel_exceptions.begin(), parcel_exceptions.end());
    }
    return all_exceptions;
}

} // namespace services
} // namespace dharti
