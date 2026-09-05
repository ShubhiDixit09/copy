#include <iostream>
#include <cassert>
#include <cmath>
#include "dharti/services/contradiction_engine.hpp"
#include "dharti/services/sia_inclusion_engine.hpp"
#include "pci_engine.h"

void test_contradiction_engine() {
    dharti::services::ContradictionEngine engine(1.0);

    // Test 1: Clean parcel
    dharti::models::ParcelVersion p1;
    p1.parcel_id = 101;
    p1.ror_area_sqm = 1000.0;
    p1.cadastral_area_sqm = 1005.0; // 0.5% diff <= 1% tolerance
    p1.ror_owner = "Ramesh Kumar";
    p1.field_claimant = "Ramesh Kumar";
    p1.has_active_stay = false;

    auto exceptions1 = engine.evaluate_parcel(p1);
    assert(exceptions1.empty());

    // Test 2: Injunction + Title Mismatch + Area Discrepancy
    dharti::models::ParcelVersion p2;
    p2.parcel_id = 118;
    p2.ror_area_sqm = 1000.0;
    p2.cadastral_area_sqm = 1150.0; // 15% diff > 1%
    p2.ror_owner = "Ramesh Kumar";
    p2.field_claimant = "Suresh Kumar"; // Conflict
    p2.has_active_stay = true;          // Court stay

    auto exceptions2 = engine.evaluate_parcel(p2);
    assert(exceptions2.size() == 3);
    std::cout << "[PASS] C++ Contradiction Engine tests passed." << std::endl;
}

void test_sia_inclusion_engine() {
    dharti::services::SIAInclusionEngine engine;

    std::vector<dharti::models::Household> universe;

    // 10 households observed
    for (int i = 1; i <= 6; ++i) {
        dharti::models::Household h;
        h.household_id = "HH-" + std::to_string(i);
        h.category = "TitleHolder";
        h.family_member_count = 4;
        h.is_vulnerable = false;
        h.has_award_mapping = true;
        h.has_rnr_mapping = false;
        h.is_reviewed_ineligible = false;
        universe.push_back(h);
    }

    // 4 vulnerable households missing from award/R&R (as in SIH demo walkthrough)
    for (int i = 7; i <= 10; ++i) {
        dharti::models::Household h;
        h.household_id = "HH-" + std::to_string(i);
        h.category = "LivelihoodDependent";
        h.family_member_count = 5;
        h.is_vulnerable = true;
        h.has_award_mapping = false;
        h.has_rnr_mapping = false;
        h.is_reviewed_ineligible = false;
        universe.push_back(h);
    }

    auto report = engine.audit_universe(universe);
    assert(report.total_households_observed == 10);
    assert(report.award_mapped_count == 6);
    assert(report.missing_unaccounted_count == 4);
    assert(!report.is_compliant);

    auto cases = engine.generate_exception_cases(report.missing_households);
    assert(cases.size() == 4);
    std::cout << "[PASS] C++ SIA Inclusion Engine tests passed (detected 4 missing vulnerable households)." << std::endl;
}

void test_pci_native() {
    ChainageInterval intervals[5] = {
        {101, 0.0, 3.0, 1},
        {102, 3.0, 5.0, 1},
        {118, 5.0, 6.5, 0},
        {104, 6.5, 10.0, 1},
        {105, 10.0, 12.0, 0}
    };

    PCIResult result;
    int code = compute_pci_native(12.0, intervals, 5, &result);
    assert(code == 0);
    assert(std::abs(result.total_ready_length - 8.5) < 1e-6);
    assert(std::abs(result.max_continuous_ready_length - 5.0) < 1e-6);

    UnlockCandidate candidates[5];
    int actual_candidates = 0;
    int sim_code = simulate_unlock_native(12.0, intervals, 5, candidates, 5, &actual_candidates);
    assert(sim_code == 0);
    assert(actual_candidates == 2);
    assert(candidates[0].parcel_id == 118);
    assert(std::abs(candidates[0].unlock_gain - 5.0) < 1e-6);

    std::cout << "[PASS] C++ Native PCI & Unlock simulation tests passed." << std::endl;
}

int main() {
    std::cout << "Running DHARTI Native C++ Test Suite..." << std::endl;
    test_contradiction_engine();
    test_sia_inclusion_engine();
    test_pci_native();
    std::cout << "All DHARTI C++ domain tests PASSED successfully!" << std::endl;
    return 0;
}
