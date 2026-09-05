#ifndef DHARTI_SERVICES_PCI_ENGINE_HPP
#define DHARTI_SERVICES_PCI_ENGINE_HPP

#include "dharti/core/base.hpp"
#include <vector>

namespace dharti {
namespace services {

struct Interval {
    int parcel_id;
    double start_chainage;
    double end_chainage;
    bool is_ready;
};

struct PCIReport {
    double total_length;
    double total_ready_length;
    double total_blocked_length;
    double max_continuous_ready_length;
    double pci;
};

struct UnlockRanking {
    int parcel_id;
    double current_max_run;
    double simulated_max_run;
    double unlock_gain;
};

/**
 * @brief C++ High-Performance Possession Continuity Index & Unlock Simulation Engine.
 */
class PCIEngine : public core::BaseService {
public:
    PCIEngine() = default;
    ~PCIEngine() override = default;

    core::ServiceHealth health_check() const override;
    std::string service_name() const override;

    PCIReport compute_pci(double total_length, const std::vector<Interval>& intervals) const;

    std::vector<UnlockRanking> simulate_unlock(
        double total_length,
        const std::vector<Interval>& intervals,
        size_t max_rankings = 10
    ) const;
};

} // namespace services
} // namespace dharti

#endif // DHARTI_SERVICES_PCI_ENGINE_HPP
