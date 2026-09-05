#include "dharti/services/workflow_coordinator.hpp"
#include "dharti/adapters/land_record_adapter.hpp"
#include "dharti/adapters/court_adapter.hpp"
#include "dharti/adapters/finance_adapter.hpp"
#include <sstream>

namespace dharti {
namespace services {

WorkflowCoordinator::WorkflowCoordinator(
    std::shared_ptr<core::BitemporalEventStore> event_store,
    std::shared_ptr<ContradictionEngine> contradiction_engine,
    std::shared_ptr<SIAInclusionEngine> sia_engine,
    std::shared_ptr<PaymentReconciler> payment_reconciler,
    std::shared_ptr<PCIEngine> pci_engine
) : event_store_(std::move(event_store)),
    contradiction_engine_(std::move(contradiction_engine)),
    sia_engine_(std::move(sia_engine)),
    payment_reconciler_(std::move(payment_reconciler)),
    pci_engine_(std::move(pci_engine)) {}

void WorkflowCoordinator::register_parcel(
    const models::ParcelVersion& parcel,
    double chainage_start,
    double chainage_end
) {
    ParcelEntry entry;
    entry.parcel = parcel;
    entry.state = models::ParcelState::CANDIDATE;
    entry.chainage_start = chainage_start;
    entry.chainage_end = chainage_end;
    entry.field_memo_verified = false;

    parcels_[parcel.parcel_id] = entry;

    if (event_store_) {
        core::BitemporalEvent ev;
        ev.event_id = "EV-REG-" + std::to_string(parcel.parcel_id);
        ev.aggregate_type = "Parcel";
        ev.aggregate_id = "P-" + std::to_string(parcel.parcel_id);
        ev.event_type = "PARCEL_REGISTERED";
        ev.payload_data = "Registered at chainage [" + std::to_string(chainage_start) + " to " + std::to_string(chainage_end) + "]";
        ev.valid_time = "2026-09-01T10:00:00Z";
        ev.transaction_time = "2026-09-01T10:05:00Z";
        ev.recorded_by = "WORKFLOW_COORDINATOR";
        event_store_->append_event(ev);
    }
}

void WorkflowCoordinator::ingest_project_package(
    const std::string& /* project_file */,
    const std::string& ror_file,
    const std::string& cadastral_file,
    const std::string& court_file,
    const std::string& pfms_file
) {
    // 1. Ingest land records and cadastral geometry
    adapters::LandRecordAdapter land_adapter("KA");
    auto land_records = land_adapter.ingest_from_files(ror_file, cadastral_file);

    // 2. Ingest court dockets
    adapters::CourtAdapter court_adapter("KARNATAKA_HC");
    auto court_records = court_adapter.ingest_from_file(court_file);

    // 3. Register each parcel
    for (const auto& lr : land_records) {
        models::ParcelVersion pv;
        pv.parcel_id = lr.parcel_id;
        pv.ulpin = lr.canonical_khasra_code;
        pv.survey_number = lr.canonical_khasra_code;
        pv.ror_area_sqm = lr.ror_area_sqm;
        pv.cadastral_area_sqm = lr.cadastral_area_sqm;
        pv.ror_owner = lr.owner_name;
        pv.field_claimant = lr.owner_name;
        pv.has_active_stay = false;

        // Check against court records
        for (const auto& cr : court_records) {
            if (cr.target_khasra_code == lr.canonical_khasra_code && cr.has_active_stay) {
                pv.has_active_stay = true;
                pv.field_claimant = "Disputed / " + lr.owner_name;
                break;
            }
        }

        register_parcel(pv, lr.chainage_start_km, lr.chainage_end_km);
    }

    // 4. Ingest finance records
    if (payment_reconciler_) {
        adapters::FinanceAdapter finance_adapter("PFMS_DIRECT");
        auto payment_records = finance_adapter.ingest_from_file(pfms_file);
        for (const auto& pr : payment_records) {
            models::PaymentRecord obligation;
            obligation.payment_id = pr.mandate_id;
            obligation.parcel_id = pr.parcel_id;
            obligation.amount_inr = pr.net_amount_inr;
            obligation.state = models::PaymentState::OBLIGATION_CREATED;
            payment_reconciler_->register_obligation(obligation);

            if (pr.is_settled || pr.is_failed) {
                payment_reconciler_->reconcile_advice(pr);
            }
        }
    }
}

StageGateResult WorkflowCoordinator::advance_to_awarded(int64_t parcel_id) {
    StageGateResult result;
    auto it = parcels_.find(parcel_id);
    if (it == parcels_.end()) {
        result.is_cleared = false;
        result.rejection_reason = "Parcel ID not found.";
        return result;
    }

    ParcelEntry& entry = it->second;

    // Check for active judicial injunctions
    if (entry.parcel.has_active_stay) {
        result.is_cleared = false;
        result.rejection_reason = "Cannot declare award: Active judicial stay order in place.";
        result.blocking_factors.push_back("JUDICIAL_STAY_ACTIVE");
        return result;
    }

    entry.state = models::ParcelState::AWARDED;
    result.is_cleared = true;

    if (event_store_) {
        core::BitemporalEvent ev;
        ev.event_id = "EV-AWD-" + std::to_string(parcel_id);
        ev.aggregate_type = "Parcel";
        ev.aggregate_id = "P-" + std::to_string(parcel_id);
        ev.event_type = "AWARD_DECLARED";
        ev.payload_data = "Statutory award declared under Section 23";
        ev.valid_time = "2026-09-02T11:00:00Z";
        ev.transaction_time = "2026-09-02T11:02:00Z";
        ev.recorded_by = "COMPETENT_AUTHORITY";
        event_store_->append_event(ev);
    }

    return result;
}

StageGateResult WorkflowCoordinator::advance_to_possession_verified(int64_t parcel_id, const std::string& field_memo_ref) {
    StageGateResult result;
    auto it = parcels_.find(parcel_id);
    if (it == parcels_.end()) {
        result.is_cleared = false;
        result.rejection_reason = "Parcel ID not found.";
        return result;
    }

    ParcelEntry& entry = it->second;

    // INVARIANT 3: Physical possession requires verified financial compensation settlement
    if (payment_reconciler_ && !payment_reconciler_->is_parcel_financially_cleared(parcel_id)) {
        result.is_cleared = false;
        result.rejection_reason = "INVARIANT-3 VIOLATION: Financial compensation not fully settled or bank payment failed.";
        auto summary = payment_reconciler_->get_parcel_summary(parcel_id);
        for (const auto& r : summary.pending_reasons) {
            result.blocking_factors.push_back(r);
        }
        return result;
    }

    // Require valid signed field memo
    if (field_memo_ref.empty()) {
        result.is_cleared = false;
        result.rejection_reason = "Field possession memo reference is missing or unsigned.";
        result.blocking_factors.push_back("FIELD_MEMO_MISSING");
        return result;
    }

    entry.state = models::ParcelState::POSSESSION_VERIFIED;
    entry.field_memo_verified = true;
    entry.memo_reference = field_memo_ref;
    result.is_cleared = true;

    if (event_store_) {
        core::BitemporalEvent ev;
        ev.event_id = "EV-POSS-" + std::to_string(parcel_id);
        ev.aggregate_type = "Parcel";
        ev.aggregate_id = "P-" + std::to_string(parcel_id);
        ev.event_type = "POSSESSION_VERIFIED";
        ev.payload_data = "Physical possession taken. Field memo ref: " + field_memo_ref;
        ev.valid_time = "2026-09-03T15:30:00Z";
        ev.transaction_time = "2026-09-03T15:32:00Z";
        ev.recorded_by = "REVENUE_COLLECTOR";
        event_store_->append_event(ev);
    }

    return result;
}

StageGateResult WorkflowCoordinator::advance_to_construction_ready(int64_t parcel_id) {
    StageGateResult result;
    auto it = parcels_.find(parcel_id);
    if (it == parcels_.end()) {
        result.is_cleared = false;
        result.rejection_reason = "Parcel ID not found.";
        return result;
    }

    ParcelEntry& entry = it->second;

    if (entry.state != models::ParcelState::POSSESSION_VERIFIED) {
        result.is_cleared = false;
        result.rejection_reason = "Parcel must reach PossessionVerified before ConstructionReady.";
        result.blocking_factors.push_back("POSSESSION_NOT_VERIFIED");
        return result;
    }

    if (entry.parcel.has_active_stay) {
        result.is_cleared = false;
        result.rejection_reason = "Active judicial stay order blocks construction handover.";
        result.blocking_factors.push_back("ACTIVE_COURT_STAY");
        return result;
    }

    // Verify no blocking contradictions remain
    if (contradiction_engine_) {
        auto contradictions = contradiction_engine_->evaluate_parcel(entry.parcel);
        if (!contradictions.empty()) {
            result.is_cleared = false;
            result.rejection_reason = "Unresolved spatial or legal contradictions exist on parcel.";
            for (const auto& c : contradictions) {
                result.blocking_factors.push_back(c.description);
            }
            return result;
        }
    }

    entry.state = models::ParcelState::CONSTRUCTION_READY;
    result.is_cleared = true;

    if (event_store_) {
        core::BitemporalEvent ev;
        ev.event_id = "EV-RDY-" + std::to_string(parcel_id);
        ev.aggregate_type = "Parcel";
        ev.aggregate_id = "P-" + std::to_string(parcel_id);
        ev.event_type = "CONSTRUCTION_HANDOVER_COMPLETE";
        ev.payload_data = "Parcel handed over to infrastructure concessionaire";
        ev.valid_time = "2026-09-04T09:00:00Z";
        ev.transaction_time = "2026-09-04T09:05:00Z";
        ev.recorded_by = "PROJECT_DIRECTOR";
        event_store_->append_event(ev);
    }

    return result;
}

void WorkflowCoordinator::resolve_court_stay(int64_t parcel_id, const std::string& vacate_order_ref) {
    auto it = parcels_.find(parcel_id);
    if (it != parcels_.end()) {
        it->second.parcel.has_active_stay = false;

        if (event_store_) {
            core::BitemporalEvent ev;
            ev.event_id = "EV-STAY-VAC-" + std::to_string(parcel_id);
            ev.aggregate_type = "Parcel";
            ev.aggregate_id = "P-" + std::to_string(parcel_id);
            ev.event_type = "STAY_ORDER_VACATED";
            ev.payload_data = "Injunction vacated by competent court order: " + vacate_order_ref;
            ev.valid_time = "2026-09-03T14:00:00Z";
            ev.transaction_time = "2026-09-03T14:10:00Z";
            ev.recorded_by = "GOVERNMENT_LEGAL_COUNSEL";
            event_store_->append_event(ev);
        }
    }
}

void WorkflowCoordinator::resolve_title_mismatch(int64_t parcel_id, const std::string& field_claimant_name) {
    auto it = parcels_.find(parcel_id);
    if (it != parcels_.end()) {
        it->second.parcel.field_claimant = field_claimant_name;

        if (event_store_) {
            core::BitemporalEvent ev;
            ev.event_id = "EV-TITLE-RES-" + std::to_string(parcel_id);
            ev.aggregate_type = "Parcel";
            ev.aggregate_id = "P-" + std::to_string(parcel_id);
            ev.event_type = "TITLE_DISPUTE_RESOLVED";
            ev.payload_data = "Succession mutation completed in favor of: " + field_claimant_name;
            ev.valid_time = "2026-09-03T11:00:00Z";
            ev.transaction_time = "2026-09-03T11:08:00Z";
            ev.recorded_by = "TALUK_TAHSILDAR";
            event_store_->append_event(ev);
        }
    }
}

void WorkflowCoordinator::harmonize_cadastral_survey(int64_t parcel_id, double verified_cadastral_sqm) {
    auto it = parcels_.find(parcel_id);
    if (it != parcels_.end()) {
        it->second.parcel.cadastral_area_sqm = verified_cadastral_sqm;

        if (event_store_) {
            core::BitemporalEvent ev;
            ev.event_id = "EV-JMS-RES-" + std::to_string(parcel_id);
            ev.aggregate_type = "Parcel";
            ev.aggregate_id = "P-" + std::to_string(parcel_id);
            ev.event_type = "JMS_SURVEY_HARMONIZED";
            ev.payload_data = "Joint Measurement Survey area fixed at: " + std::to_string(verified_cadastral_sqm) + " sqm";
            ev.valid_time = "2026-09-03T12:00:00Z";
            ev.transaction_time = "2026-09-03T12:05:00Z";
            ev.recorded_by = "DIRECTOR_OF_SURVEY";
            event_store_->append_event(ev);
        }
    }
}

PCIReport WorkflowCoordinator::evaluate_corridor_pci(double total_corridor_length_km) const {
    if (!pci_engine_) {
        return {};
    }

    std::vector<Interval> intervals;
    for (const auto& pair : parcels_) {
        const auto& entry = pair.second;
        bool is_ready = (entry.state == models::ParcelState::CONSTRUCTION_READY);
        intervals.push_back({entry.parcel.parcel_id, entry.chainage_start, entry.chainage_end, is_ready});
    }

    return pci_engine_->compute_pci(total_corridor_length_km, intervals);
}

std::vector<UnlockRanking> WorkflowCoordinator::identify_unlock_priorities(double total_corridor_length_km, size_t top_k) const {
    if (!pci_engine_) {
        return {};
    }

    std::vector<Interval> intervals;
    for (const auto& pair : parcels_) {
        const auto& entry = pair.second;
        bool is_ready = (entry.state == models::ParcelState::CONSTRUCTION_READY);
        intervals.push_back({entry.parcel.parcel_id, entry.chainage_start, entry.chainage_end, is_ready});
    }

    return pci_engine_->simulate_unlock(total_corridor_length_km, intervals, top_k);
}

models::ParcelState WorkflowCoordinator::get_parcel_state(int64_t parcel_id) const {
    auto it = parcels_.find(parcel_id);
    if (it != parcels_.end()) {
        return it->second.state;
    }
    return models::ParcelState::CANDIDATE;
}

models::ParcelVersion WorkflowCoordinator::get_parcel(int64_t parcel_id) const {
    auto it = parcels_.find(parcel_id);
    if (it != parcels_.end()) {
        return it->second.parcel;
    }
    return {};
}

} // namespace services
} // namespace dharti
