#include <iostream>
#include <cassert>
#include <cmath>
#include <cstdlib>

#include "dharti/config/settings.hpp"
#include "dharti/utils/logger.hpp"
#include "dharti/models/parcel.hpp"
#include "dharti/models/claimant.hpp"
#include "dharti/models/payment.hpp"
#include "dharti/services/contradiction_engine.hpp"
#include "dharti/services/sia_inclusion_engine.hpp"
#include "dharti/services/pci_engine.hpp"

using namespace dharti;

void test_config() {
    auto& cfg = config::AppConfig::instance();
    assert(cfg.app_name() == "DHARTI");
    assert(cfg.app_env() == "development");
    assert(cfg.debug() == true);
    assert(cfg.log_level() == "INFO");
    std::cout << "[PASS] C++ AppConfig test passed." << std::endl;
}

void test_logger() {
    auto logger = utils::get_logger("TestLogger");
    assert(logger.name() == "TestLogger");
    logger.info("Verifying C++ structured logging");
    std::cout << "[PASS] C++ Logger test passed." << std::endl;
}

void test_models_and_state_machines() {
    using namespace models;
    assert(std::string(parcel_state_to_string(ParcelState::CANDIDATE)) == "Candidate");
    assert(std::string(parcel_state_to_string(ParcelState::CONSTRUCTION_READY)) == "ConstructionReady");
    assert(std::string(claimant_state_to_string(ClaimantState::OBSERVED_IN_SIA)) == "ObservedInSIA");
    assert(std::string(claimant_state_to_string(ClaimantState::RESOLVED)) == "Resolved");
    assert(std::string(payment_state_to_string(PaymentState::OBLIGATION_CREATED)) == "ObligationCreated");
    assert(std::string(payment_state_to_string(PaymentState::RECEIPT_CONFIRMED)) == "ReceiptConfirmed");
    std::cout << "[PASS] C++ Domain Models & State Machines test passed." << std::endl;
}

void test_contradiction_engine() {
    services::ContradictionEngine engine(1.0);

    // Clean parcel
    models::ParcelVersion p1;
    p1.parcel_id = 101;
    p1.ror_area_sqm = 1000.0;
    p1.cadastral_area_sqm = 1005.0; // 0.5% diff <= 1%
    p1.ror_owner = "Ramesh Kumar";
    p1.field_claimant = "Ramesh Kumar";
    p1.has_active_stay = false;
    assert(engine.evaluate_parcel(p1).empty());

    // Conflicted parcel (Stay + Title Mismatch + Area Discrepancy > 1%)
    models::ParcelVersion p2;
    p2.parcel_id = 118;
    p2.ror_area_sqm = 1000.0;
    p2.cadastral_area_sqm = 1150.0; // 15% diff > 1%
    p2.ror_owner = "Ramesh Kumar";
    p2.field_claimant = "Suresh Kumar";
    p2.has_active_stay = true;

    auto exceptions = engine.evaluate_parcel(p2);
    assert(exceptions.size() == 3);
    std::cout << "[PASS] C++ Contradiction Engine test passed (3/3 exceptions detected)." << std::endl;
}

void test_sia_inclusion_engine() {
    services::SIAInclusionEngine engine;
    std::vector<models::Household> universe;

    // 6 mapped households
    for (int i = 1; i <= 6; ++i) {
        models::Household h;
        h.household_id = "HH-" + std::to_string(i);
        h.category = "TitleHolder";
        h.family_member_count = 4;
        h.is_vulnerable = false;
        h.has_award_mapping = true;
        h.has_rnr_mapping = false;
        h.is_reviewed_ineligible = false;
        universe.push_back(h);
    }

    // 4 vulnerable households missing from award/R&R
    for (int i = 7; i <= 10; ++i) {
        models::Household h;
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
    std::cout << "[PASS] C++ SIA Inclusion Engine test passed (4/4 missing vulnerable households flagged)." << std::endl;
}

void test_pci_engine() {
    services::PCIEngine engine;
    std::vector<services::Interval> intervals = {
        {101, 0.0, 3.0, true},
        {102, 3.0, 5.0, true},
        {118, 5.0, 6.5, false}, // Bottleneck parcel P-118
        {104, 6.5, 10.0, true},
        {105, 10.0, 12.0, false}
    };

    auto report = engine.compute_pci(12.0, intervals);
    assert(std::abs(report.total_ready_length - 8.5) < 1e-6);
    assert(std::abs(report.total_blocked_length - 3.5) < 1e-6);
    assert(std::abs(report.max_continuous_ready_length - 5.0) < 1e-6);
    assert(std::abs(report.pci - (5.0 / 12.0)) < 1e-6);

    auto rankings = engine.simulate_unlock(12.0, intervals, 5);
    assert(rankings.size() == 2);
    assert(rankings[0].parcel_id == 118);
    assert(std::abs(rankings[0].simulated_max_run - 10.0) < 1e-6);
    assert(std::abs(rankings[0].unlock_gain - 5.0) < 1e-6);

    assert(rankings[1].parcel_id == 105);
    assert(std::abs(rankings[1].unlock_gain - 0.5) < 1e-6);

    std::cout << "[PASS] C++ PCI Engine & Unlock Simulation test passed (P-118 yields +5.0 km continuous frontage)." << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "   DHARTI Pure C++ Domain Test Suite   " << std::endl;
    std::cout << "========================================" << std::endl;

    test_config();
    test_logger();
    test_models_and_state_machines();
    test_contradiction_engine();
    test_sia_inclusion_engine();
    test_pci_engine();

    std::cout << "========================================" << std::endl;
    std::cout << "   ALL C++ TESTS PASSED (100% GREEN)   " << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
