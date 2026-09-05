#ifndef DHARTI_SERVICES_SIA_INCLUSION_ENGINE_HPP
#define DHARTI_SERVICES_SIA_INCLUSION_ENGINE_HPP

#include "dharti/core/base.hpp"
#include "dharti/models/claimant.hpp"
#include "dharti/models/contradiction.hpp"
#include <vector>

namespace dharti {
namespace services {

struct SIAAuditReport {
    int total_households_observed;
    int award_mapped_count;
    int rnr_mapped_count;
    int lawfully_excluded_count;
    int missing_unaccounted_count;
    bool is_compliant; // True if missing_unaccounted_count == 0
    std::vector<models::Household> missing_households;
};

/**
 * @brief Enforces the "No Family Invisible" rule from RFCTLARR / World Bank ESS5 standards.
 */
class SIAInclusionEngine : public core::BaseService {
public:
    SIAInclusionEngine() = default;
    ~SIAInclusionEngine() override = default;

    core::ServiceHealth health_check() const override;
    std::string service_name() const override;

    /**
     * @brief Audits the entire surveyed SIA universe against award and R&R entitlement mappings.
     */
    SIAAuditReport audit_universe(const std::vector<models::Household>& universe) const;

    /**
     * @brief Converts unmapped households into actionable contradiction exception cases.
     */
    std::vector<models::ContradictionCase> generate_exception_cases(
        const std::vector<models::Household>& missing_households
    ) const;

    /**
     * @brief Ingests surveyed households from real SIA JSON file.
     */
    std::vector<models::Household> ingest_from_file(const std::string& sia_file_path) const;
};

} // namespace services
} // namespace dharti

#endif // DHARTI_SERVICES_SIA_INCLUSION_ENGINE_HPP
