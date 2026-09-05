#ifndef DHARTI_SERVICES_CONTRADICTION_ENGINE_HPP
#define DHARTI_SERVICES_CONTRADICTION_ENGINE_HPP

#include "dharti/core/base.hpp"
#include "dharti/models/parcel.hpp"
#include "dharti/models/contradiction.hpp"
#include <vector>

namespace dharti {
namespace services {

/**
 * @brief Deterministic cross-source contradiction engine.
 * Invariant: Never silently merges conflicts; creates explicit quarantined exception items.
 */
class ContradictionEngine : public core::BaseService {
public:
    ContradictionEngine(double area_tolerance_percent = 1.0);
    ~ContradictionEngine() override = default;

    core::ServiceHealth health_check() const override;
    std::string service_name() const override;

    /**
     * @brief Evaluates a parcel against RoR, cadastral map, and court registries.
     * @return Vector of detected contradiction exception cases.
     */
    std::vector<models::ContradictionCase> evaluate_parcel(
        const models::ParcelVersion& parcel
    ) const;

    /**
     * @brief Batch evaluation across a portfolio of parcels.
     */
    std::vector<models::ContradictionCase> evaluate_portfolio(
        const std::vector<models::ParcelVersion>& parcels
    ) const;

private:
    double area_tolerance_percent_;
};

} // namespace services
} // namespace dharti

#endif // DHARTI_SERVICES_CONTRADICTION_ENGINE_HPP
