#include "dharti/services/pci_engine.hpp"
#include "pci_engine.h"
#include <vector>
#include <stdexcept>

namespace dharti {
namespace services {

core::ServiceHealth PCIEngine::health_check() const {
    return core::ServiceHealth{
        service_name(),
        core::HealthStatus::HEALTHY,
        "PCI Computation & Unlock Simulation Engine operational",
        1757088000000ULL
    };
}

std::string PCIEngine::service_name() const {
    return "PCIEngine";
}

PCIReport PCIEngine::compute_pci(
    double total_length,
    const std::vector<Interval>& intervals
) const {
    if (total_length <= 0.0) {
        throw std::invalid_argument("total_length must be positive");
    }

    std::vector<ChainageInterval> c_intervals;
    c_intervals.reserve(intervals.size());
    for (const auto& iv : intervals) {
        c_intervals.push_back(ChainageInterval{
            iv.parcel_id,
            iv.start_chainage,
            iv.end_chainage,
            iv.is_ready ? 1 : 0
        });
    }

    PCIResult res;
    int code = compute_pci_native(total_length, c_intervals.data(), static_cast<int>(c_intervals.size()), &res);
    if (code != 0) {
        throw std::runtime_error("compute_pci_native failed");
    }

    return PCIReport{
        res.total_length,
        res.total_ready_length,
        res.total_blocked_length,
        res.max_continuous_ready_length,
        res.pci
    };
}

std::vector<UnlockRanking> PCIEngine::simulate_unlock(
    double total_length,
    const std::vector<Interval>& intervals,
    size_t max_rankings
) const {
    if (total_length <= 0.0) {
        throw std::invalid_argument("total_length must be positive");
    }

    std::vector<ChainageInterval> c_intervals;
    c_intervals.reserve(intervals.size());
    for (const auto& iv : intervals) {
        c_intervals.push_back(ChainageInterval{
            iv.parcel_id,
            iv.start_chainage,
            iv.end_chainage,
            iv.is_ready ? 1 : 0
        });
    }

    std::vector<UnlockCandidate> c_candidates(intervals.size() + 1);
    int actual_count = 0;

    int code = simulate_unlock_native(
        total_length,
        c_intervals.data(),
        static_cast<int>(c_intervals.size()),
        c_candidates.data(),
        static_cast<int>(c_candidates.size()),
        &actual_count
    );

    if (code != 0) {
        throw std::runtime_error("simulate_unlock_native failed");
    }

    std::vector<UnlockRanking> rankings;
    size_t count = std::min(static_cast<size_t>(actual_count), max_rankings);
    for (size_t i = 0; i < count; ++i) {
        rankings.push_back(UnlockRanking{
            c_candidates[i].parcel_id,
            c_candidates[i].current_max_run,
            c_candidates[i].simulated_max_run,
            c_candidates[i].unlock_gain
        });
    }

    return rankings;
}

} // namespace services
} // namespace dharti
