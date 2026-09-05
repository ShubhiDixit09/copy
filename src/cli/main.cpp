#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <iomanip>
#include <sstream>

#include "dharti/utils/json.hpp"
#include "dharti/utils/logger.hpp"
#include "dharti/config/settings.hpp"
#include "dharti/models/parcel.hpp"
#include "dharti/models/claimant.hpp"
#include "dharti/models/contradiction.hpp"
#include "dharti/core/event_store.hpp"
#include "dharti/adapters/land_record_adapter.hpp"
#include "dharti/adapters/court_adapter.hpp"
#include "dharti/adapters/finance_adapter.hpp"
#include "dharti/services/contradiction_engine.hpp"
#include "dharti/services/sia_inclusion_engine.hpp"
#include "dharti/services/pci_engine.hpp"
#include "dharti/services/payment_reconciler.hpp"
#include "dharti/services/workflow_coordinator.hpp"

using namespace dharti;

void print_banner() {
    std::cout << "================================================================================" << std::endl;
    std::cout << "   DHARTI: National Land Acquisition Control Plane (SIH 26016)                 " << std::endl;
    std::cout << "   Real-Time Generalized Evidence Engine & Corridor Analytics                  " << std::endl;
    std::cout << "================================================================================" << std::endl;
}

void run_real_benchmark(size_t N) {
    std::cout << "\n>>> EXECUTING REALTIME EMPIRICAL WALL-CLOCK BENCHMARK (N=" << N << ") <<<" << std::endl;
    services::PCIEngine engine;
    std::vector<services::Interval> intervals;
    intervals.reserve(N);

    // Generate real deterministic interval chain
    double cur = 0.0;
    for (size_t i = 1; i <= N; ++i) {
        double len = 0.5 + static_cast<double>((i * 7919) % 250) / 100.0;
        bool ready = (i % 7 != 0); // 85% ready, 15% blocked
        intervals.push_back({static_cast<int>(i), cur, cur + len, ready});
        cur += len;
    }
    double total_corridor = cur;

    // Measure PCI Calculation Latency
    const int RUNS = 100;
    auto t0 = std::chrono::high_resolution_clock::now();
    services::PCIReport report;
    for (int r = 0; r < RUNS; ++r) {
        report = engine.compute_pci(total_corridor, intervals);
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    double pci_us = std::chrono::duration<double, std::micro>(t1 - t0).count() / RUNS;

    std::cout << "  - Total Generated Corridor Length: " << std::fixed << std::setprecision(2) << total_corridor << " km" << std::endl;
    std::cout << "  - Total Ready Intervals: " << report.total_ready_length << " km" << std::endl;
    std::cout << "  - Longest Continuous Frontage: " << report.max_continuous_ready_length << " km" << std::endl;
    std::cout << "  - Measured Corridor PCI: " << (report.pci * 100.0) << "%" << std::endl;
    std::cout << "  - Real PCI Merging Latency (" << RUNS << " runs mean): " << std::setprecision(3) << pci_us << " microseconds ("
              << (pci_us / 1000.0) << " ms)" << std::endl;

    // Measure Bottleneck Unlock Simulation Latency
    size_t k_sample = std::min<size_t>(10, N / 5 + 1);
    auto u0 = std::chrono::high_resolution_clock::now();
    auto rankings = engine.simulate_unlock(total_corridor, intervals, k_sample);
    auto u1 = std::chrono::high_resolution_clock::now();
    double unlock_us = std::chrono::duration<double, std::micro>(u1 - u0).count();

    std::cout << "  - Real Unlock Ranking Latency (Top-" << k_sample << "): " << unlock_us << " microseconds ("
              << (unlock_us / 1000.0) << " ms)" << std::endl;
    if (!rankings.empty()) {
        std::cout << "  - Top Bottleneck Parcel: P-" << rankings[0].parcel_id
                  << " | Unlock Gain: +" << rankings[0].unlock_gain << " km continuous frontage" << std::endl;
    }
    std::cout << ">>> BENCHMARK COMPLETE <<<\n" << std::endl;
}

int main(int argc, char* argv[]) {
    print_banner();

    std::string project_file = "data/projects/nhai_corridor_12km.json";
    std::string ror_file = "data/ror/karnataka_revenue_ror.json";
    std::string cadastral_file = "data/cadastral/bhoomi_cadastral_parcels.json";
    std::string court_file = "data/ecourts/high_court_dockets.json";
    std::string pfms_file = "data/pfms/treasury_payment_advices.json";
    std::string sia_file = "data/sia/sia_household_census.json";
    double corridor_km = 12.0;
    bool run_bench = false;
    size_t bench_n = 1000;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--project" && i + 1 < argc) project_file = argv[++i];
        else if (arg == "--ror" && i + 1 < argc) ror_file = argv[++i];
        else if (arg == "--cadastral" && i + 1 < argc) cadastral_file = argv[++i];
        else if (arg == "--court" && i + 1 < argc) court_file = argv[++i];
        else if (arg == "--pfms" && i + 1 < argc) pfms_file = argv[++i];
        else if (arg == "--sia" && i + 1 < argc) sia_file = argv[++i];
        else if (arg == "--corridor-km" && i + 1 < argc) corridor_km = std::stod(argv[++i]);
        else if (arg == "--benchmark" && i + 1 < argc) { run_bench = true; bench_n = std::stoull(argv[++i]); }
    }

    if (run_bench) {
        run_real_benchmark(bench_n);
        return 0;
    }

    std::cout << "==> Initializing DHARTI Control Plane Components..." << std::endl;
    auto event_store = std::make_shared<core::BitemporalEventStore>();
    auto contradiction_engine = std::make_shared<services::ContradictionEngine>(1.0);
    auto sia_engine = std::make_shared<services::SIAInclusionEngine>();
    auto payment_reconciler = std::make_shared<services::PaymentReconciler>();
    auto pci_engine = std::make_shared<services::PCIEngine>();

    services::WorkflowCoordinator coordinator(
        event_store,
        contradiction_engine,
        sia_engine,
        payment_reconciler,
        pci_engine
    );

    std::cout << "==> Ingesting Real Data Files from Disk:" << std::endl;
    std::cout << "    [Project]   " << project_file << std::endl;
    std::cout << "    [RoR]       " << ror_file << std::endl;
    std::cout << "    [Cadastral] " << cadastral_file << std::endl;
    std::cout << "    [Court]     " << court_file << std::endl;
    std::cout << "    [Finance]   " << pfms_file << std::endl;
    std::cout << "    [SIA]       " << sia_file << std::endl;

    try {
        coordinator.ingest_project_package(
            project_file,
            ror_file,
            cadastral_file,
            court_file,
            pfms_file
        );
        std::cout << "[SUCCESS] Ingestion completed successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Ingestion failed: " << e.what() << std::endl;
        return 1;
    }

    // Evaluate SIA universe
    std::cout << "\n==> Running SIA Household Inclusion Audit..." << std::endl;
    auto households = sia_engine->ingest_from_file(sia_file);
    auto sia_report = sia_engine->audit_universe(households);
    std::cout << "    - Total Surveyed Households: " << sia_report.total_households_observed << std::endl;
    std::cout << "    - Mapped to Section 23 Award: " << sia_report.award_mapped_count << std::endl;
    std::cout << "    - Unmapped Vulnerable Families: " << sia_report.missing_unaccounted_count << std::endl;
    std::cout << "    - Invariant 4 Compliance: " << (sia_report.is_compliant ? "PASSED" : "FAILED (CIRCUIT BREAKER ENGAGED)") << std::endl;

    // Evaluate initial corridor PCI
    std::cout << "\n==> Computing Corridor Possession Continuity Index (PCI)..." << std::endl;
    auto pci_report = coordinator.evaluate_corridor_pci(corridor_km);
    std::cout << "    - Alignment Total Length: " << corridor_km << " km" << std::endl;
    std::cout << "    - Total Evidence-Ready Run: " << pci_report.total_ready_length << " km" << std::endl;
    std::cout << "    - Longest Continuous Frontage: " << pci_report.max_continuous_ready_length << " km" << std::endl;
    std::cout << "    - Current PCI: " << std::fixed << std::setprecision(2) << (pci_report.pci * 100.0) << "%" << std::endl;

    // Identify real bottlenecks
    std::cout << "\n==> Simulating Corridor Bottleneck Unlock Priorities..." << std::endl;
    auto priorities = coordinator.identify_unlock_priorities(corridor_km, 5);
    for (size_t i = 0; i < priorities.size(); ++i) {
        std::cout << "    #" << (i + 1) << " Parcel ID: P-" << priorities[i].parcel_id
                  << " | Current Max Run: " << priorities[i].current_max_run << " km"
                  << " -> Simulated Max Run: " << priorities[i].simulated_max_run << " km"
                  << " (Gain: +" << priorities[i].unlock_gain << " km continuous frontage)" << std::endl;
    }

    // Verify Ledger
    std::cout << "\n==> Cryptographic Audit Ledger Status:" << std::endl;
    std::cout << "    - Total Events Recorded: " << event_store->event_count() << std::endl;
    std::cout << "    - Cryptographic Chain Verification: "
              << (event_store->verify_ledger_integrity() ? "100% UNBROKEN INTEGRITY" : "TAMPERED / BROKEN") << std::endl;

    std::cout << "\n================================================================================" << std::endl;
    std::cout << "   DHARTI Real-Time Execution Completed Successfully.                           " << std::endl;
    std::cout << "================================================================================" << std::endl;
    return 0;
}
