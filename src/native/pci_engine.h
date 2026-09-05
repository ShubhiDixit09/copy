#ifndef DHARTI_PCI_ENGINE_H
#define DHARTI_PCI_ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
  #define DHARTI_API __declspec(dllexport)
#else
  #define DHARTI_API __attribute__((visibility("default")))
#endif

typedef struct {
    int parcel_id;          // Unique numerical ID of the parcel
    double start_chainage;  // Start chainage in meters or kilometers
    double end_chainage;    // End chainage in meters or kilometers
    int is_ready;           // 1 = evidence-ready, 0 = blocked
} ChainageInterval;

typedef struct {
    double total_length;
    double total_ready_length;
    double total_blocked_length;
    double max_continuous_ready_length;
    double pci;             // max_continuous_ready_length / total_length
} PCIResult;

typedef struct {
    int parcel_id;
    double current_max_run;
    double simulated_max_run;
    double unlock_gain;     // simulated_max_run - current_max_run
} UnlockCandidate;

/**
 * Computes Possession Continuity Index (PCI) and continuous constructible frontage.
 * @param total_length Total corridor alignment length.
 * @param intervals Array of chainage intervals.
 * @param count Number of elements in intervals array.
 * @param out_result Pointer to PCIResult struct populated with output metrics.
 * @return 0 on success, non-zero on error.
 */
DHARTI_API int compute_pci_native(
    double total_length,
    const ChainageInterval* intervals,
    int count,
    PCIResult* out_result
);

/**
 * Simulates flipping each blocked parcel to ready and ranks candidates by unlock gain.
 * @param total_length Total corridor alignment length.
 * @param intervals Array of chainage intervals.
 * @param count Number of elements in intervals array.
 * @param out_candidates Pre-allocated array to receive ranked candidates.
 * @param max_candidates Capacity of out_candidates array.
 * @param actual_candidate_count Pointer to int receiving total candidates populated.
 * @return 0 on success, non-zero on error.
 */
DHARTI_API int simulate_unlock_native(
    double total_length,
    const ChainageInterval* intervals,
    int count,
    UnlockCandidate* out_candidates,
    int max_candidates,
    int* actual_candidate_count
);

#ifdef __cplusplus
}
#endif

#endif // DHARTI_PCI_ENGINE_H
