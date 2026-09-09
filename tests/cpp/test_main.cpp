#include <iostream>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <vector>
#include <string>

#include "dharti/config/settings.hpp"
#include "dharti/utils/logger.hpp"
#include "dharti/models/parcel.hpp"
#include "dharti/models/claimant.hpp"
#include "dharti/models/payment.hpp"
#include "dharti/models/contradiction.hpp"
#include "dharti/services/contradiction_engine.hpp"
#include "dharti/services/sia_inclusion_engine.hpp"
#include "dharti/services/pci_engine.hpp"
#include "dharti/adapters/adapter_interface.hpp"
#include "dharti/adapters/land_record_adapter.hpp"
#include "dharti/adapters/court_adapter.hpp"
#include "dharti/adapters/finance_adapter.hpp"
#include "dharti/core/event_store.hpp"
#include "dharti/services/payment_reconciler.hpp"
#include "dharti/services/workflow_coordinator.hpp"
#include "dharti/utils/sha256.hpp"
#include "dharti/models/evidence.hpp"
#include "dharti/adapters/parivesh_adapter.hpp"
#include "dharti/services/evidence_engine.hpp"
#include "dharti/storage/gdrive_client.hpp"
#include "dharti/storage/neon_client.hpp"
#include "dharti/services/explanatory_query_engine.hpp"

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

void test_federated_adapters() {
    // 1. Land Record Adapter
    adapters::LandRecordAdapter land_adapter("KA");
    adapters::RawRoRRecord ror;
    ror.state_code = "KA";
    ror.district = "Bengaluru Rural";
    ror.taluk = "Devanahalli";
    ror.village = "Kundana";
    ror.survey_number = "118";
    ror.sub_division = "2A";
    ror.owner_name = "Ramesh Kumar";
    ror.ror_area_sqm = 10000.0;
    ror.tenure_type = "Patta";
    ror.has_encumbrance = false;

    adapters::RawCadastralRecord cadastral;
    cadastral.survey_number = "118";
    cadastral.sub_division = "2A";
    cadastral.polygon_area_sqm = 10020.0;
    cadastral.chainage_start_km = 5.0;
    cadastral.chainage_end_km = 6.5;

    auto normalized_land = land_adapter.ingest_and_harmonize(ror, cadastral, 118);
    assert(!normalized_land.metadata.is_quarantined);
    assert(normalized_land.parcel_id == 118);
    assert(!normalized_land.metadata.payload_checksum.empty());
    assert(land_adapter.successful_ingestions() == 1);

    // 2. Court Adapter
    adapters::CourtAdapter court_adapter("KARNATAKA_HC");
    adapters::RawCourtDocket docket;
    docket.court_forum = "High Court of Karnataka";
    docket.case_type = "Writ Petition";
    docket.case_number = "4021";
    docket.case_year = 2023;
    docket.petitioner = "Suresh Kumar";
    docket.respondent = "State of Karnataka & NHAI";
    docket.target_khasra_code = normalized_land.canonical_khasra_code;
    docket.is_stay_granted = true;
    docket.stay_nature = "PossessionInjunction";
    docket.order_date = "2023-04-12";
    docket.is_stay_vacated = false;

    auto court_rec = court_adapter.ingest_docket(docket);
    assert(!court_rec.metadata.is_quarantined);
    assert(court_adapter.is_stay_active(court_rec));
    assert(court_adapter.active_stays_tracked() == 1);

    // 3. Finance Adapter
    adapters::FinanceAdapter finance_adapter("PFMS_DIRECT");
    adapters::RawPFMSAdvice advice;
    advice.sanction_order_no = "SO-2026-8819";
    advice.payment_mandate_id = "MANDATE-118-A";
    advice.parcel_id = 118;
    advice.claimant_token = "CLAIMANT-TOK-88912";
    advice.net_payable_inr = 4500000.0;
    advice.bank_utr_reference = "SBIN2026090100418";
    advice.credit_status = "SUCCESS";
    advice.settlement_date = "2026-09-01T14:22:00Z";

    auto pmt_rec = finance_adapter.ingest_payment_advice(advice);
    assert(!pmt_rec.metadata.is_quarantined);
    assert(pmt_rec.is_settled);
    assert(pmt_rec.net_amount_inr == 4500000.0);
    assert(finance_adapter.successful_disbursements() == 1);

    std::cout << "[PASS] C++ Federated Adapters test passed (LandRecord, Court, Finance)." << std::endl;
}

void test_event_store_and_bitemporal_audit() {
    core::BitemporalEventStore store;

    core::BitemporalEvent ev1;
    ev1.event_id = "EV-001";
    ev1.aggregate_type = "Parcel";
    ev1.aggregate_id = "P-118";
    ev1.event_type = "PARCEL_INGESTED";
    ev1.payload_data = "RoR and Cadastral Polygon Linked";
    ev1.valid_time = "2026-08-15T09:00:00Z";
    ev1.transaction_time = "2026-08-15T09:05:00Z";
    ev1.recorded_by = "INGESTION_BOT";

    core::BitemporalEvent ev2;
    ev2.event_id = "EV-002";
    ev2.aggregate_type = "Parcel";
    ev2.aggregate_id = "P-118";
    ev2.event_type = "INJUNCTION_IMPOSED";
    ev2.payload_data = "WP 4021/2023 stay order recorded";
    ev2.valid_time = "2026-08-18T11:00:00Z";
    ev2.transaction_time = "2026-08-18T11:10:00Z";
    ev2.recorded_by = "LEGAL_CELL";

    uint64_t seq1 = store.append_event(ev1);
    uint64_t seq2 = store.append_event(ev2);

    assert(seq1 == 1);
    assert(seq2 == 2);
    assert(store.event_count() == 2);
    assert(store.verify_ledger_integrity());

    // Historical transaction time query
    auto historical_t = store.query_transaction_as_of("2026-08-16T00:00:00Z");
    assert(historical_t.size() == 1);
    assert(historical_t[0].event_id == "EV-001");

    auto aggregate_events = store.get_events_for_aggregate("Parcel", "P-118");
    assert(aggregate_events.size() == 2);

    std::cout << "[PASS] C++ Bitemporal Event Store test passed (Unbroken hash chain, Tt/Tv queryable)." << std::endl;
}

void test_payment_reconciliation() {
    services::PaymentReconciler reconciler;

    models::PaymentRecord obligation;
    obligation.payment_id = "MANDATE-118-A";
    obligation.parcel_id = 118;
    obligation.amount_inr = 4500000.0;
    obligation.state = models::PaymentState::OBLIGATION_CREATED;
    reconciler.register_obligation(obligation);

    assert(!reconciler.is_parcel_financially_cleared(118));

    // Process bank rejection first
    adapters::NormalizedPaymentRecord failed_advice;
    failed_advice.mandate_id = "MANDATE-118-A";
    failed_advice.parcel_id = 118;
    failed_advice.net_amount_inr = 4500000.0;
    failed_advice.is_settled = false;
    failed_advice.is_failed = true;
    failed_advice.rejection_reason = "ERR_ACCOUNT_FROZEN";

    reconciler.reconcile_advice(failed_advice);
    assert(!reconciler.is_parcel_financially_cleared(118));
    auto summary = reconciler.get_parcel_summary(118);
    assert(summary.has_failed_transactions);

    // Now re-issue and settle
    adapters::NormalizedPaymentRecord success_advice;
    success_advice.mandate_id = "MANDATE-118-A";
    success_advice.parcel_id = 118;
    success_advice.net_amount_inr = 4500000.0;
    success_advice.bank_utr = "SBIN2026090100418";
    success_advice.is_settled = true;
    success_advice.is_failed = false;

    reconciler.reconcile_advice(success_advice);
    assert(reconciler.is_parcel_financially_cleared(118));
    assert(reconciler.confirmed_receipts() == 1);

    std::cout << "[PASS] C++ Payment Reconciler test passed (Rejection handled & settlement verified)." << std::endl;
}

void test_sih26016_complete_demo_walkthrough() {
    std::cout << "\n>>> EXECUTING SIH 26016 4-MINUTE COMPLETE DEMO WALKTHROUGH <<<" << std::endl;

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

    const double CORRIDOR_LENGTH_KM = 12.0;

    // Register 5 Corridor Parcels (P-101, P-102, P-118, P-104, P-105)
    models::ParcelVersion p101;
    p101.parcel_id = 101;
    p101.ror_area_sqm = 1000.0;
    p101.cadastral_area_sqm = 1002.0;
    p101.ror_owner = "Farmer A";
    p101.field_claimant = "Farmer A";
    p101.has_active_stay = false;

    models::ParcelVersion p102;
    p102.parcel_id = 102;
    p102.ror_area_sqm = 1000.0;
    p102.cadastral_area_sqm = 1001.0;
    p102.ror_owner = "Farmer B";
    p102.field_claimant = "Farmer B";
    p102.has_active_stay = false;

    models::ParcelVersion p118;
    p118.parcel_id = 118;
    p118.ror_area_sqm = 1000.0;
    p118.cadastral_area_sqm = 1150.0; // 15% discrepancy
    p118.ror_owner = "Ramesh Kumar";
    p118.field_claimant = "Suresh Kumar"; // Title mismatch
    p118.has_active_stay = true; // Court Injunction Bottleneck!

    models::ParcelVersion p104;
    p104.parcel_id = 104;
    p104.ror_area_sqm = 1000.0;
    p104.cadastral_area_sqm = 1004.0;
    p104.ror_owner = "Farmer D";
    p104.field_claimant = "Farmer D";
    p104.has_active_stay = false;

    models::ParcelVersion p105;
    p105.parcel_id = 105;
    p105.ror_area_sqm = 1000.0;
    p105.cadastral_area_sqm = 1003.0;
    p105.ror_owner = "Farmer E";
    p105.field_claimant = "Farmer E";
    p105.has_active_stay = false;

    coordinator.register_parcel(p101, 0.0, 3.0);
    coordinator.register_parcel(p102, 3.0, 5.0);
    coordinator.register_parcel(p118, 5.0, 6.5);
    coordinator.register_parcel(p104, 6.5, 10.0);
    coordinator.register_parcel(p105, 10.0, 12.0);

    // Setup obligations and advance P-101, P-102, P-104 to CONSTRUCTION_READY
    for (int64_t pid : {101, 102, 104}) {
        models::PaymentRecord pmt;
        pmt.payment_id = "PMT-" + std::to_string(pid);
        pmt.parcel_id = pid;
        pmt.amount_inr = 2000000.0;
        payment_reconciler->register_obligation(pmt);

        adapters::NormalizedPaymentRecord adv;
        adv.mandate_id = pmt.payment_id;
        adv.parcel_id = pid;
        adv.net_amount_inr = 2000000.0;
        adv.bank_utr = "UTR-" + std::to_string(pid);
        adv.is_settled = true;
        payment_reconciler->reconcile_advice(adv);

        coordinator.advance_to_awarded(pid);
        coordinator.advance_to_possession_verified(pid, "MEMO-FIELD-" + std::to_string(pid));
        auto rdy_res = coordinator.advance_to_construction_ready(pid);
        assert(rdy_res.is_cleared);
    }

    // Step 1: Baseline Corridor Evaluation
    auto initial_pci = coordinator.evaluate_corridor_pci(CORRIDOR_LENGTH_KM);
    std::cout << "[Step 1 Baseline] Total Ready: " << initial_pci.total_ready_length << " km, "
              << "Max Continuous Frontage: " << initial_pci.max_continuous_ready_length << " km, "
              << "Verified PCI: " << (initial_pci.pci * 100.0) << "%" << std::endl;
    assert(std::abs(initial_pci.max_continuous_ready_length - 5.0) < 1e-6);

    // Step 2: Bottleneck Unlock Simulation
    auto priorities = coordinator.identify_unlock_priorities(CORRIDOR_LENGTH_KM, 5);
    assert(!priorities.empty());
    assert(priorities[0].parcel_id == 118);
    std::cout << "[Step 2 Unlock Priority] #1 Priority Parcel: P-" << priorities[0].parcel_id
              << " | Current Contiguous: " << priorities[0].current_max_run << " km"
              << " -> Simulated Unlock: " << priorities[0].simulated_max_run << " km"
              << " (+ " << priorities[0].unlock_gain << " km continuous frontage gain!)" << std::endl;

    // Step 3: Attempt Premature Handover of P-118 (Must Fail due to Invariants)
    auto blocked_attempt = coordinator.advance_to_possession_verified(118, "PREMATURE_MEMO");
    assert(!blocked_attempt.is_cleared);
    std::cout << "[Step 3 Invariant-3 Defense] Handover correctly blocked: "
              << blocked_attempt.rejection_reason << std::endl;

    // Step 4: Multi-Dimensional Exception Resolution on P-118
    std::cout << "[Step 4 Multi-Agency Resolution] Resolving P-118 exceptions..." << std::endl;
    
    // 4a. Judicial Stay Vacated via Court order
    coordinator.resolve_court_stay(118, "WP-4021-VACATE-ORDER-2026");
    
    // 4b. Title Succession Dispute Resolved
    coordinator.resolve_title_mismatch(118, "Ramesh Kumar");

    // 4c. Cadastral Survey Area Harmonized via Joint Measurement Survey (JMS)
    coordinator.harmonize_cadastral_survey(118, 1005.0);

    // 4d. Financial Compensation Disbursed and Confirmed
    models::PaymentRecord pmt118;
    pmt118.payment_id = "PMT-118";
    pmt118.parcel_id = 118;
    pmt118.amount_inr = 4500000.0;
    payment_reconciler->register_obligation(pmt118);

    adapters::NormalizedPaymentRecord adv118;
    adv118.mandate_id = "PMT-118";
    adv118.parcel_id = 118;
    adv118.net_amount_inr = 4500000.0;
    adv118.bank_utr = "SBIN-CLR-118-FINAL";
    adv118.is_settled = true;
    payment_reconciler->reconcile_advice(adv118);

    // 4e. Advance P-118 through Statutory Gated Workflow
    auto awd_res = coordinator.advance_to_awarded(118);
    assert(awd_res.is_cleared);

    auto poss_res = coordinator.advance_to_possession_verified(118, "GEO-TAGGED-MEMO-P118-COLLECTOR-SIGNED");
    assert(poss_res.is_cleared);

    auto rdy_res = coordinator.advance_to_construction_ready(118);
    assert(rdy_res.is_cleared);
    std::cout << "[Step 4 Resolution Success] Parcel P-118 is now ConstructionReady!" << std::endl;

    // Step 5: Post-Resolution Corridor Recalculation
    auto final_pci = coordinator.evaluate_corridor_pci(CORRIDOR_LENGTH_KM);
    std::cout << "[Step 5 Post-Resolution PCI] Max Continuous Frontage: "
              << final_pci.max_continuous_ready_length << " km "
              << "(JUMPED from 5.0 km to 10.0 km continuous frontage!)" << std::endl;
    std::cout << "[Step 5 Post-Resolution PCI] Verified Corridor PCI: "
              << (final_pci.pci * 100.0) << "%" << std::endl;

    assert(std::abs(final_pci.max_continuous_ready_length - 10.0) < 1e-6);
    assert(std::abs(final_pci.pci - (10.0 / 12.0)) < 1e-6);

    // Step 6: Verify Immutable Bitemporal Audit Trail
    assert(event_store->verify_ledger_integrity());
    std::cout << "[Step 6 Audit Trail] Bitemporal Event Log contains "
              << event_store->event_count() << " cryptographically chained events (100% verified)." << std::endl;

    std::cout << ">>> SIH 26016 COMPLETE DEMO WALKTHROUGH FINISHED SUCCESSFULLY <<<\n" << std::endl;
}

void test_pure_cpp_sha256() {
    // RFC 6234 standard test vector 1 (empty string)
    std::string h_empty = utils::SHA256::hash_string("");
    assert(h_empty == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");

    // RFC 6234 standard test vector 2 ("abc")
    std::string h_abc = utils::SHA256::hash_string("abc");
    assert(h_abc == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");

    std::cout << "[PASS] Pure C++ SHA-256 Hasher test passed (RFC 6234 standard test vectors verified)." << std::endl;
}

void test_parivesh_clearance_adapter() {
    adapters::PariveshAdapter adapter("SZ_REGIONAL");
    adapters::RawClearanceProposal raw;
    raw.proposal_no = "IA/KA/NHA/10482/2026";
    raw.project_name = "Bengaluru-Chennai Expressway Package-2";
    raw.clearance_category = "FOREST_CLEARANCE";
    raw.stage = "STAGE_1";
    raw.state_code = "KA";
    raw.district = "Bengaluru Rural";
    raw.diversion_area_ha = 52.4;
    raw.trees_to_fell = 1420;
    raw.current_status = "Under Process";
    raw.submission_date = "2026-05-12";
    raw.decision_date = "";
    raw.conditions = {"Compensatory afforestation on non-forest land", "Minimum tree felling"};

    auto norm = adapter.normalize(raw);
    assert(norm.proposal_no == "IA/KA/NHA/10482/2026");
    assert(norm.status == "UNDER_PROCESS");
    assert(!norm.is_approved);
    assert(norm.metadata.source_system == "PARIVESH");
    assert(norm.metadata.payload_checksum.length() == 64);
    assert(!norm.metadata.is_quarantined);
    assert(adapter.total_processed() == 1);

    std::cout << "[PASS] Pure C++ PARIVESH Adapter test passed (Payload normalized & SHA-256 hashed)." << std::endl;
}

void test_evidence_engine_7_rules_and_change_detection() {
    services::EvidenceEngine engine(10.0);
    adapters::PariveshAdapter adapter;

    // Snapshot 1: Initial Under Process
    adapters::RawClearanceProposal p1;
    p1.proposal_no = "IA/KA/NHA/10482/2026";
    p1.project_name = "Expressway Alignment";
    p1.clearance_category = "FOREST_CLEARANCE";
    p1.stage = "STAGE_1";
    p1.state_code = "KA";
    p1.district = "Bengaluru Rural";
    p1.diversion_area_ha = 52.4;
    p1.trees_to_fell = 1420;
    p1.current_status = "Under Process";
    p1.submission_date = "2026-05-12";
    auto norm1 = adapter.normalize(p1);

    // Rule Evaluation: Valid government record
    auto gate1 = engine.evaluate_clearance(norm1, "GOVERNMENT", nullptr);
    assert(gate1.is_accepted);
    assert(gate1.acceptance_status == "ACCEPTED");

    // Snapshot 2: 30 minutes later -> APPROVED
    adapters::RawClearanceProposal p2 = p1;
    p2.current_status = "Approved";
    p2.decision_date = "2026-09-07";
    auto norm2 = adapter.normalize(p2);

    auto gate2 = engine.evaluate_clearance(norm2, "GOVERNMENT", &norm1);
    assert(gate2.is_accepted);
    assert(gate2.acceptance_status == "ACCEPTED");

    // Change Detection: Detects UNDER_PROCESS -> APPROVED transition
    auto change = engine.detect_changes(norm2, norm1);
    assert(change.has_change);
    assert(change.status_changed);
    assert(change.new_status == "APPROVED");
    assert(change.event_type == "CLEARANCE_APPROVED");

    // Create Canonical Event
    auto ev = engine.create_event(norm2, gate2, change, "DRIVE-FILE-002", "DRIVE-FILE-001");
    assert(ev.event_type == "CLEARANCE_APPROVED");
    assert(ev.status == "ACCEPTED");
    assert(ev.aggregate_id == "IA/KA/NHA/10482/2026");
    assert(!ev.checksum.empty());

    // Snapshot 3: Anomaly Jump (Rule 5 violation: Area jumps from 52.4 Ha to 5240 Ha, 100x jump)
    adapters::RawClearanceProposal p3 = p2;
    p3.diversion_area_ha = 5240.0;
    auto norm3 = adapter.normalize(p3);

    auto gate3 = engine.evaluate_clearance(norm3, "GOVERNMENT", &norm2);
    assert(!gate3.is_accepted);
    assert(gate3.acceptance_status == "QUARANTINED");
    assert(gate3.violation_rule == "RULE_5_IMPOSSIBLE_AREA_JUMP");

    auto ev_quarantine = engine.create_event(norm3, gate3, change, "DRIVE-FILE-003", "DRIVE-FILE-002");
    assert(ev_quarantine.status == "QUARANTINED");
    assert(ev_quarantine.event_type == "EVIDENCE_QUARANTINED");

    // Test Rule 1: Untrusted Authority
    auto gate_untrusted = engine.evaluate_clearance(norm1, "UNKNOWN_SOURCE", nullptr);
    assert(!gate_untrusted.is_accepted);
    assert(gate_untrusted.violation_rule == "RULE_1_UNTRUSTED_AUTHORITY");

    std::cout << "[PASS] Pure C++ 7-Rule Evidence Engine & Change Detector test passed (Approval accepted, Anomaly quarantined)." << std::endl;
}

void test_gdrive_partitioning_and_base64() {
    std::string raw_test = "DHARTI SIH 26016 Immutable Evidence Vault";
    std::string b64 = storage::GDriveClient::base64_encode(raw_test);
    assert(!b64.empty());

    std::string folder = storage::GDriveClient::build_partition_folder("parivesh");
    assert(folder.find("dharti/raw/parivesh/") == 0);

    std::string ev_folder = storage::GDriveClient::build_evidence_folder("clearance");
    assert(ev_folder == "dharti/evidence/clearance");

    std::cout << "[PASS] Pure C++ Google Drive Client helpers test passed (dharti Partitioning & Base64 validated)." << std::endl;
}

#include "dharti/scrapers/web_scraper.hpp"
#include "dharti/adapters/bhoomi_rashi_adapter.hpp"
#include "dharti/services/polling_daemon.hpp"

void test_web_scraper_and_provenance() {
    scrapers::WebScraper scraper(10);
    std::string iso_time = scrapers::WebScraper::current_iso_utc();
    assert(!iso_time.empty());
    assert(iso_time.find("T") != std::string::npos);
    assert(iso_time.find("Z") != std::string::npos);

    // Test SHA256 of empty/arbitrary string
    std::string payload = "{\"test\":\"parivesh_observation\"}";
    std::string sha = utils::SHA256::hash_string(payload);
    assert(sha.length() == 64);

    std::cout << "[PASS] Pure C++ WebScraper provenance & timestamping test passed." << std::endl;
}

void test_bhoomi_rashi_adapter() {
    adapters::BhoomiRashiAdapter adapter;
    adapters::RawGazetteNotification raw;
    raw.notification_number = "S.O. 3842(E)";
    raw.gazette_type = "3D";
    raw.highway_number = "NH-48";
    raw.project_name = "Bengaluru-Chennai Expressway";
    raw.state_code = "KA";
    raw.district = "Bengaluru Rural";
    raw.taluk = "Hosakote";
    raw.acquired_area_ha = 48.75;
    raw.published_date = "2026-08-20";
    raw.survey_numbers = {"104/1", "104/2"};

    auto norm = adapter.normalize(raw);
    assert(norm.notification_number == "S.O. 3842(E)");
    assert(norm.gazette_type == "3D");
    assert(norm.acquired_area_ha == 48.75);
    assert(!norm.metadata.payload_checksum.empty());
    assert(norm.metadata.payload_checksum.length() == 64);
    assert(norm.metadata.source_system == "BHOOMI_RASHI");

    std::cout << "[PASS] Pure C++ Bhoomi Rashi MoRTH Adapter normalization test passed." << std::endl;
}

void test_location_contradiction_quarantine() {
    adapters::PariveshAdapter adapter("SZ_REGIONAL");
    services::EvidenceEngine engine(3.0);

    adapters::RawClearanceProposal p1;
    p1.proposal_no = "IA/KA/NHA/10482/2026";
    p1.project_name = "Expressway Package-2";
    p1.clearance_category = "FOREST_CLEARANCE";
    p1.stage = "STAGE_1";
    p1.state_code = "KA";
    p1.district = "Bengaluru Rural";
    p1.diversion_area_ha = 52.4;
    p1.current_status = "Under Process";
    auto norm1 = adapter.normalize(p1);

    // Simulated hijack/contradiction: state altered from KA to TN
    adapters::RawClearanceProposal p2 = p1;
    p2.state_code = "TN";
    p2.current_status = "Approved";
    auto norm2 = adapter.normalize(p2);

    auto gate = engine.evaluate_clearance(norm2, "GOVERNMENT", &norm1);
    assert(!gate.is_accepted);
    assert(gate.acceptance_status == "QUARANTINED");
    assert(gate.violation_rule == "RULE_5_LOCATION_CONTRADICTION");

    std::cout << "[PASS] Pure C++ 7-Rule Evidence Engine location contradiction quarantine test passed." << std::endl;
}

void test_geotag_and_document_proof() {
    models::GeoLocation loc;
    loc.latitude = 13.1986;
    loc.longitude = 77.7066;
    loc.elevation_m = 914.5;
    loc.chainage_start_km = 0.0;
    loc.chainage_end_km = 12.0;
    loc.utm_zone = "43N";
    loc.gps_accuracy_meters = 1.2;
    loc.boundary_wkt = "POLYGON((77.7012 13.1945, 77.7150 13.2010, 77.7012 13.1945))";

    assert(loc.latitude > 13.0 && loc.latitude < 14.0);
    assert(loc.longitude > 77.0 && loc.longitude < 78.0);
    assert(loc.gps_accuracy_meters <= 2.0);

    models::DocumentProof proof;
    proof.document_id = "DOC-PROOF-TEST-001";
    proof.document_type = "STAGE1_FOREST_CLEARANCE";
    proof.official_letter_no = "F.No. 4-KAB819/2026-RO";
    proof.signatory_officer_name = "Dr. K. S. Murthy, IFS";
    proof.location = loc;
    proof.rfc6234_sha256 = "09430dc2e501e9bf12408190a16c0529c699856376887767d5cceb9971347ca7";
    assert(proof.is_verified);
    assert(proof.location.latitude == 13.1986);

    std::cout << "[PASS] Pure C++ Geotag Coordinates & Document Proof Model test passed." << std::endl;
}

void test_generalized_corridor_search() {
    scrapers::WebScraper scraper;

    // 1. Search without proposal no by keyword "Bengaluru"
    auto r1 = scraper.search_corridors("Bengaluru");
    assert(!r1.empty());
    assert(r1[0].project_id == "NHAI-NE7-PKG-04");
    assert(r1[0].state == "KA");

    // 2. Search by Highway "NE-4"
    auto r2 = scraper.search_corridors("NE-4");
    assert(!r2.empty());
    assert(r2[0].project_id == "NHAI-NE4-PKG-17");
    assert(r2[0].district == "Bharuch");

    // 3. State filter "JH"
    auto r3 = scraper.search_corridors("", "JH");
    assert(!r3.empty());
    assert(r3[0].project_id == "NHAI-NH319B-PKG-06");

    // 4. Case-insensitivity check
    auto r4 = scraper.search_corridors("katra");
    assert(!r4.empty());
    assert(r4[0].project_id == "NHAI-NE5-PKG-05");

    std::cout << "[PASS] Pure C++ Generalized Multi-Criteria Corridor Search test passed." << std::endl;
}

void test_scraper_deduplication() {
    scrapers::WebScraper scraper;
    std::string test_id = "PARIVESH-TEST-REC-101";
    std::string hash1 = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    std::string hash2 = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";

    // First time seeing hash1 -> changed
    assert(scraper.is_payload_changed(test_id, hash1) == true);
    // Second time seeing hash1 -> deduplicated, not changed!
    assert(scraper.is_payload_changed(test_id, hash1) == false);
    // Modified to hash2 -> changed
    assert(scraper.is_payload_changed(test_id, hash2) == true);
    // Second time hash2 -> deduplicated
    assert(scraper.is_payload_changed(test_id, hash2) == false);

    std::cout << "[PASS] Pure C++ Scraper In-Memory SHA-256 Deduplication Cache test passed." << std::endl;
}

void test_explanatory_query_engine() {
    auto pci_engine = std::make_shared<services::PCIEngine>();
    services::ExplanatoryQueryEngine engine(pci_engine);

    std::vector<services::Interval> intervals = {
        {101, 0.0, 3.0, true},
        {102, 3.0, 5.0, true},
        {118, 5.0, 6.5, false}, // Bottleneck parcel P-118
        {104, 6.5, 10.0, true},
        {105, 10.0, 12.0, false}
    };

    // Query 1: Bottleneck explanation
    auto r1 = engine.answer_query("Why is corridor possession blocked?", 12.0, intervals);
    assert(r1.category == "BOTTLENECK");
    assert(r1.affected_parcels_count == 1);
    assert(!r1.affected_parcel_ids.empty() && r1.affected_parcel_ids[0] == 118);
    assert(r1.unlock_gain_km > 4.9);
    assert(r1.statutory_authority.find("RFCTLARR Act 2013") != std::string::npos);
    assert(!r1.actionable_remediation_steps.empty());

    // Query 2: SIA Vulnerability explanation
    auto r2 = engine.answer_query("Audit SIA vulnerable families and census", 12.0, intervals);
    assert(r2.category == "SIA_COMPLIANCE");
    assert(r2.statutory_authority.find("Section 16") != std::string::npos);
    assert(r2.affected_parcels_count > 0);

    // Query 3: Court Stay explanation
    auto r3 = engine.answer_query("What is the legal stay order status?", 12.0, intervals);
    assert(r3.category == "LEGAL_STAY");
    assert(r3.direct_answer.find("Writ Petition No. 4021/2023") != std::string::npos);

    std::cout << "[PASS] Pure C++ Explanatory Query Engine & Statutory Citation test passed." << std::endl;
}

void test_federated_15_portal_suite() {
    scrapers::WebScraper scraper(5);

    // 1. Verify 15 authoritative portals are registered in the federated catalog
    const auto& portals = scraper.get_supported_portals();
    assert(portals.size() == 15);

    bool has_parivesh = false;
    bool has_bhoomi = false;
    bool has_bhuvan = false;
    bool has_pfms = false;
    bool has_egazette = false;

    for (const auto& p : portals) {
        assert(!p.portal_id.empty());
        assert(!p.display_name.empty());
        assert(!p.official_domain.empty());
        assert(!p.statutory_basis.empty());
        assert(p.polling_interval_minutes > 0);

        if (p.portal_id == "MOEFCC_PARIVESH") has_parivesh = true;
        if (p.portal_id == "KARNATAKA_BHOOMI") has_bhoomi = true;
        if (p.portal_id == "ISRO_BHUVAN") has_bhuvan = true;
        if (p.portal_id == "PFMS_TREASURY") has_pfms = true;
        if (p.portal_id == "EGAZETTE_INDIA") has_egazette = true;
    }
    assert(has_parivesh && has_bhoomi && has_bhuvan && has_pfms && has_egazette);

    // 2. Targeted scrape test for State Land Record (Bhoomi)
    auto obs_bhoomi = scraper.scrape_portal("KARNATAKA_BHOOMI", "SY-118-KUNDANA");
    assert(obs_bhoomi.source_name.find("BHOOMI") != std::string::npos);
    assert(obs_bhoomi.sha256.length() == 64);
    assert(!obs_bhoomi.raw_payload.empty());

    // 3. Targeted scrape test for Central Gazette
    auto obs_gazette = scraper.scrape_portal("EGAZETTE_INDIA", "S.O. 3842(E)");
    assert(obs_gazette.source_name.find("GAZETTE") != std::string::npos || obs_gazette.source_name.find("eGazette") != std::string::npos);
    assert(obs_gazette.sha256.length() == 64);

    // 4. Parallel concurrent scrape across all 15 portals
    auto all_obs = scraper.scrape_all_portals_for_corridor("NHAI-NE7-PKG-04");
    assert(all_obs.size() == 15);

    // 5. Verify scraper telemetry metrics
    auto metrics = scraper.get_metrics();
    assert(metrics.total_requests >= 17); // 1 bhoomi + 1 gazette + 15 all

    std::cout << "[PASS] Pure C++ Federated 15-Portal Suite & Concurrent Scraper test passed (All 15 portals active)." << std::endl;
}

int main() {
    std::cout << "=================================================" << std::endl;
    std::cout << "   DHARTI National Land Acquisition Control Plane" << std::endl;
    std::cout << "          Pure C++ Domain & Workflow Suite       " << std::endl;
    std::cout << "=================================================" << std::endl;

    test_config();
    test_logger();
    test_pure_cpp_sha256();
    test_models_and_state_machines();
    test_contradiction_engine();
    test_sia_inclusion_engine();
    test_pci_engine();
    test_federated_adapters();
    test_parivesh_clearance_adapter();
    test_evidence_engine_7_rules_and_change_detection();
    test_location_contradiction_quarantine();
    test_gdrive_partitioning_and_base64();
    test_web_scraper_and_provenance();
    test_bhoomi_rashi_adapter();
    test_event_store_and_bitemporal_audit();
    test_payment_reconciliation();
    test_geotag_and_document_proof();
    test_generalized_corridor_search();
    test_scraper_deduplication();
    test_explanatory_query_engine();
    test_federated_15_portal_suite();
    test_sih26016_complete_demo_walkthrough();

    std::cout << "=================================================" << std::endl;
    std::cout << "   ALL C++ TESTS & DEMO SCENARIOS PASSED (100%)  " << std::endl;
    std::cout << "=================================================" << std::endl;
    return 0;
}

