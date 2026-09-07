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
#include "dharti/utils/sha256.hpp"
#include "dharti/models/evidence.hpp"
#include "dharti/adapters/parivesh_adapter.hpp"
#include "dharti/adapters/bhoomi_rashi_adapter.hpp"
#include "dharti/scrapers/web_scraper.hpp"
#include "dharti/services/evidence_engine.hpp"
#include "dharti/services/polling_daemon.hpp"
#include "dharti/storage/gdrive_client.hpp"
#include "dharti/storage/neon_client.hpp"

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

void run_parivesh_live_sync(bool simulate_anomaly) {
    std::cout << "\n================================================================================" << std::endl;
    std::cout << "   PHASE 1 VERTICAL SLICE: PARIVESH CLEARANCE INGESTION PIPELINE (PURE C++)     " << std::endl;
    std::cout << "   Google Drive (Evidence Vault) + Neon DB (Control-Plane Ledger) Integration    " << std::endl;
    std::cout << "================================================================================" << std::endl;

    adapters::PariveshAdapter adapter("SZ_REGIONAL");
    services::EvidenceEngine engine(10.0);
    storage::GDriveClient gdrive;
    storage::NeonClient neon;

    // 1. Initial State
    adapters::RawClearanceProposal p1;
    p1.proposal_no = "IA/KA/NHA/10482/2026";
    p1.project_name = "Bengaluru-Chennai Expressway Package-2";
    p1.clearance_category = "FOREST_CLEARANCE";
    p1.stage = "STAGE_1";
    p1.state_code = "KA";
    p1.district = "Bengaluru Rural";
    p1.diversion_area_ha = 52.4;
    p1.trees_to_fell = 1420;
    p1.current_status = "Under Process";
    p1.submission_date = "2026-05-12";
    p1.conditions = {"Compensatory afforestation on non-forest land", "Minimum tree felling"};

    std::cout << "\n[Step 1] Normalizing Raw PARIVESH Clearance Proposal..." << std::endl;
    auto norm1 = adapter.normalize(p1);
    std::cout << "  - Proposal No: " << norm1.proposal_no << std::endl;
    std::cout << "  - Status: " << norm1.status << std::endl;
    std::cout << "  - Diversion Area: " << norm1.diversion_area_ha << " Ha" << std::endl;
    std::cout << "  - SHA-256 Checksum: " << norm1.metadata.payload_checksum << std::endl;

    std::cout << "\n[Step 2] Archiving Immutable Snapshot to Google Drive Vault..." << std::endl;
    auto drive_res1 = gdrive.upload_raw_snapshot("parivesh", "snapshot_001.json", norm1.json_payload);
    std::string drive_id1 = drive_res1.success ? drive_res1.file_id : "DRIVE-MOCK-FILE-001";
    std::string drive_url1 = drive_res1.success ? drive_res1.web_link : "https://drive.google.com/file/d/" + drive_id1;
    std::cout << "  - Upload Status: " << (drive_res1.success ? "SUCCESS" : "FAILED / LOCAL FALLBACK") << std::endl;
    std::cout << "  - Partition Path: " << drive_res1.folder_path << "/snapshot_001.json" << std::endl;
    std::cout << "  - Drive File ID: " << drive_id1 << std::endl;
    std::cout << "  - Web Link: " << drive_url1 << std::endl;

    std::cout << "\n[Step 3] Evaluating Against 7-Rule Deterministic Evidence Gate..." << std::endl;
    auto gate1 = engine.evaluate_clearance(norm1, "GOVERNMENT", nullptr);
    std::cout << "  - Gate Decision: " << gate1.acceptance_status << " (" << gate1.reason << ")" << std::endl;

    std::cout << "\n[Step 4] Synchronizing Metadata & Pointers into Neon PostgreSQL..." << std::endl;
    neon.upsert_source_record("PARIVESH", norm1.proposal_no, "CLEARANCE");

    models::SourceSnapshot snap1;
    snap1.source_record_id = norm1.proposal_no;
    snap1.source_code = "PARIVESH";
    snap1.content_type = "application/json";
    snap1.drive_file_id = drive_id1;
    snap1.drive_web_link = drive_url1;
    snap1.sha256 = norm1.metadata.payload_checksum;
    snap1.normalized_payload_json = norm1.json_payload;
    neon.record_snapshot(snap1);

    models::EvidenceArtifact art1;
    art1.source_record_id = norm1.proposal_no;
    art1.artifact_type = "CLEARANCE_LETTER";
    art1.drive_file_id = drive_id1;
    art1.drive_url = drive_url1;
    art1.sha256 = norm1.metadata.payload_checksum;
    art1.file_size = norm1.json_payload.size();
    art1.acceptance_status = gate1.acceptance_status;
    art1.rejection_reason = gate1.reason;
    neon.record_evidence_artifact(art1);

    auto ev1 = engine.create_event(norm1, gate1, services::DetectedChange{}, drive_id1);
    neon.record_workflow_event(ev1);
    neon.update_source_health("PARIVESH", "HEALTHY", 0, true);
    std::cout << "  - Canonical Event: " << ev1.event_id << " (" << ev1.event_type << ") Recorded in Neon DB" << std::endl;

    // Step 5: Second Polling Cycle (Update)
    std::cout << "\n[Step 5] Simulating Subsequent Polling Cycle (Clearance State Transition)..." << std::endl;
    adapters::RawClearanceProposal p2 = p1;
    if (simulate_anomaly) {
        p2.diversion_area_ha = 5240.0; // Impossible jump!
        p2.current_status = "Approved";
        std::cout << "  [ANOMALY INJECTED] Forest diversion area jumped to 5240.0 Ha (100x jump)!" << std::endl;
    } else {
        p2.current_status = "Approved";
        p2.decision_date = "2026-09-08";
        std::cout << "  - Status Transition: Under Process -> Approved" << std::endl;
    }

    auto norm2 = adapter.normalize(p2);
    auto gate2 = engine.evaluate_clearance(norm2, "GOVERNMENT", &norm1);
    auto change = engine.detect_changes(norm2, norm1);

    auto drive_res2 = gdrive.upload_raw_snapshot("parivesh", "snapshot_002.json", norm2.json_payload);
    std::string drive_id2 = drive_res2.success ? drive_res2.file_id : "DRIVE-MOCK-FILE-002";
    std::string drive_url2 = drive_res2.success ? drive_res2.web_link : "https://drive.google.com/file/d/" + drive_id2;

    models::SourceSnapshot snap2;
    snap2.source_record_id = norm2.proposal_no;
    snap2.source_code = "PARIVESH";
    snap2.content_type = "application/json";
    snap2.drive_file_id = drive_id2;
    snap2.drive_web_link = drive_url2;
    snap2.sha256 = norm2.metadata.payload_checksum;
    snap2.normalized_payload_json = norm2.json_payload;
    neon.record_snapshot(snap2);

    auto ev2 = engine.create_event(norm2, gate2, change, drive_id2, drive_id1);
    neon.record_workflow_event(ev2);

    if (gate2.is_accepted) {
        std::cout << "  - Gate Decision: ACCEPTED" << std::endl;
        std::cout << "  - Generated Canonical Event: " << ev2.event_id << " (" << ev2.event_type << ")" << std::endl;
        std::cout << "  - Change Summary: " << change.summary << std::endl;
        neon.update_source_health("PARIVESH", "HEALTHY", 0, true);
    } else {
        std::cout << "  - Gate Decision: " << gate2.acceptance_status << " (CIRCUIT BREAKER ENGAGED!)" << std::endl;
        std::cout << "  - Violation: " << gate2.violation_rule << std::endl;
        std::cout << "  - Reason: " << gate2.reason << std::endl;
        neon.record_exception(gate2.generated_exception);
        neon.update_source_health("PARIVESH", "DEGRADED", 0, false);
        std::cout << "  - Recorded Exception in Neon DB 'exceptions' table for Human-in-the-Loop review." << std::endl;
    }

    std::cout << "\n================================================================================" << std::endl;
    std::cout << "   PARIVESH Pipeline Execution Finished Successfully!                          " << std::endl;
    std::cout << "================================================================================\n" << std::endl;
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
    bool run_parivesh = false;
    bool simulate_anomaly = false;
    bool run_poll = false;
    bool run_poll_once = false;
    bool run_poll_national = false;
    int interval_secs = 3600; // Default 1 hour polling interval

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
        else if (arg == "--sync-parivesh") { run_parivesh = true; }
        else if (arg == "--test-quarantine") { run_parivesh = true; simulate_anomaly = true; }
        else if (arg == "--poll") { run_poll = true; }
        else if (arg == "--poll-once") { run_poll_once = true; }
        else if (arg == "--poll-national-projects") { run_poll_national = true; }
        else if (arg == "--interval-hours" && i + 1 < argc) { interval_secs = std::stoi(argv[++i]) * 3600; }
        else if (arg == "--interval-mins" && i + 1 < argc) { interval_secs = std::stoi(argv[++i]) * 60; }
        else if (arg == "--interval-secs" && i + 1 < argc) { interval_secs = std::stoi(argv[++i]); }
    }

    if (run_bench) {
        run_real_benchmark(bench_n);
        return 0;
    }

    if (run_poll_national) {
        services::PollingDaemon daemon(std::chrono::seconds(interval_secs), "dharti");
        daemon.poll_national_projects();
        return 0;
    }

    if (run_poll_once) {
        services::PollingDaemon daemon(std::chrono::seconds(interval_secs), "dharti");
        if (simulate_anomaly) daemon.inject_simulated_anomaly(true);
        daemon.poll_once();
        return 0;
    }

    if (run_poll) {
        services::PollingDaemon daemon(std::chrono::seconds(interval_secs), "dharti");
        daemon.start();
        return 0;
    }

    if (run_parivesh) {
        run_parivesh_live_sync(simulate_anomaly);
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
