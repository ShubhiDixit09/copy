#include "pci_engine.h"
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <cmath>

namespace {

struct RawInterval {
    double start;
    double end;
};

// Merges sorted intervals and returns {total_ready_length, max_continuous_length}
std::pair<double, double> merge_and_calculate_runs(std::vector<RawInterval>& intervals) {
    if (intervals.empty()) {
        return {0.0, 0.0};
    }

    std::sort(intervals.begin(), intervals.end(), [](const RawInterval& a, const RawInterval& b) {
        if (std::abs(a.start - b.start) > 1e-9) {
            return a.start < b.start;
        }
        return a.end < b.end;
    });

    double total_ready = 0.0;
    double max_run = 0.0;

    double current_start = intervals[0].start;
    double current_end = intervals[0].end;

    for (size_t i = 1; i < intervals.size(); ++i) {
        if (intervals[i].start <= current_end + 1e-9) {
            // Overlapping or contiguous interval
            current_end = std::max(current_end, intervals[i].end);
        } else {
            // Discontinuity encountered
            double run_len = current_end - current_start;
            total_ready += run_len;
            if (run_len > max_run) {
                max_run = run_len;
            }
            current_start = intervals[i].start;
            current_end = intervals[i].end;
        }
    }

    // Final segment
    double last_run = current_end - current_start;
    total_ready += last_run;
    if (last_run > max_run) {
        max_run = last_run;
    }

    return {total_ready, max_run};
}

} // anonymous namespace

extern "C" {

DHARTI_API int compute_pci_native(
    double total_length,
    const ChainageInterval* intervals,
    int count,
    PCIResult* out_result
) {
    if (!out_result || total_length <= 0.0) {
        return -1;
    }

    std::vector<RawInterval> ready_intervals;
    ready_intervals.reserve(count);

    for (int i = 0; i < count; ++i) {
        if (intervals[i].is_ready != 0 && intervals[i].end_chainage > intervals[i].start_chainage) {
            ready_intervals.push_back({intervals[i].start_chainage, intervals[i].end_chainage});
        }
    }

    auto [total_ready, max_run] = merge_and_calculate_runs(ready_intervals);

    out_result->total_length = total_length;
    out_result->total_ready_length = total_ready;
    out_result->total_blocked_length = std::max(0.0, total_length - total_ready);
    out_result->max_continuous_ready_length = max_run;
    out_result->pci = (total_length > 0.0) ? (max_run / total_length) : 0.0;

    return 0;
}

DHARTI_API int simulate_unlock_native(
    double total_length,
    const ChainageInterval* intervals,
    int count,
    UnlockCandidate* out_candidates,
    int max_candidates,
    int* actual_candidate_count
) {
    if (!out_candidates || !actual_candidate_count || total_length <= 0.0) {
        return -1;
    }

    // Baseline calculation
    PCIResult base_result;
    if (compute_pci_native(total_length, intervals, count, &base_result) != 0) {
        return -1;
    }

    double baseline_max_run = base_result.max_continuous_ready_length;

    // Collect all distinct blocked parcel IDs
    std::unordered_set<int> blocked_parcels;
    for (int i = 0; i < count; ++i) {
        if (intervals[i].is_ready == 0) {
            blocked_parcels.insert(intervals[i].parcel_id);
        }
    }

    std::vector<UnlockCandidate> candidates;
    candidates.reserve(blocked_parcels.size());

    // For each blocked parcel, simulate flipping to ready
    for (int pid : blocked_parcels) {
        std::vector<RawInterval> simulated_ready;
        simulated_ready.reserve(count);

        for (int i = 0; i < count; ++i) {
            if ((intervals[i].is_ready != 0 || intervals[i].parcel_id == pid) &&
                intervals[i].end_chainage > intervals[i].start_chainage) {
                simulated_ready.push_back({intervals[i].start_chainage, intervals[i].end_chainage});
            }
        }

        auto [sim_ready, sim_max_run] = merge_and_calculate_runs(simulated_ready);
        double gain = sim_max_run - baseline_max_run;

        candidates.push_back({pid, baseline_max_run, sim_max_run, gain});
    }

    // Rank candidates: highest gain first; tie-break on lower parcel_id
    std::sort(candidates.begin(), candidates.end(), [](const UnlockCandidate& a, const UnlockCandidate& b) {
        if (std::abs(a.unlock_gain - b.unlock_gain) > 1e-9) {
            return a.unlock_gain > b.unlock_gain;
        }
        return a.parcel_id < b.parcel_id;
    });

    int return_count = std::min(static_cast<int>(candidates.size()), max_candidates);
    for (int i = 0; i < return_count; ++i) {
        out_candidates[i] = candidates[i];
    }
    *actual_candidate_count = return_count;

    return 0;
}

} // extern "C"
