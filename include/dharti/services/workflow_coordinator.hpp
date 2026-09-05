#ifndef DHARTI_SERVICES_WORKFLOW_COORDINATOR_HPP
#define DHARTI_SERVICES_WORKFLOW_COORDINATOR_HPP

#include "dharti/models/parcel.hpp"
#include "dharti/models/claimant.hpp"
#include "dharti/models/contradiction.hpp"
#include "dharti/core/event_store.hpp"
#include "dharti/services/contradiction_engine.hpp"
#include "dharti/services/sia_inclusion_engine.hpp"
#include "dharti/services/payment_reconciler.hpp"
#include "dharti/services/pci_engine.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace dharti {
namespace services {

/**
 * @brief Statutory Stage Enum (1 to 8) representing the end-to-end statutory pipeline.
 */
enum class StatutoryStage {
    STAGE_1_INITIATE,            // Alignment Freeze & Sec 11 Notification
    STAGE_2_SURVEY,              // JMS & Cadastral Geometry Freezing
    STAGE_3_INQUIRE,             // Sec 15 Hearing & SIA Inquiry
    STAGE_4_AWARD,               // Sec 19 Declaration & Sec 23 Award
    STAGE_5_COMPENSATE,          // Compensation Disbursement & R&R
    STAGE_6_DISPUTE_RESOLUTION,  // Court Injunctions & Succession Resolution
    STAGE_7_POSSESS,             // Physical Field Possession Memo
    STAGE_8_HANDOVER             // Contractor Handover & PCI Assurance
};

/**
 * @brief Gate evaluation result for parcel progression.
 */
struct StageGateResult {
    bool is_cleared{false};
    std::string rejection_reason;
    std::vector<std::string> blocking_factors;
};

/**
 * @brief Workflow Coordinator Service orchestrating the statutory land acquisition pipeline.
 */
class WorkflowCoordinator {
public:
    WorkflowCoordinator(
        std::shared_ptr<core::BitemporalEventStore> event_store,
        std::shared_ptr<ContradictionEngine> contradiction_engine,
        std::shared_ptr<SIAInclusionEngine> sia_engine,
        std::shared_ptr<PaymentReconciler> payment_reconciler,
        std::shared_ptr<PCIEngine> pci_engine
    );
    ~WorkflowCoordinator() = default;

    /**
     * @brief Register a new parcel into the control plane.
     */
    void register_parcel(const models::ParcelVersion& parcel, double chainage_start, double chainage_end);

    /**
     * @brief Advance parcel through statutory stages with invariant gating.
     */
    StageGateResult advance_to_awarded(int64_t parcel_id);
    StageGateResult advance_to_possession_verified(int64_t parcel_id, const std::string& field_memo_ref);
    StageGateResult advance_to_construction_ready(int64_t parcel_id);

    /**
     * @brief Resolve active contradiction or court stay on a parcel.
     */
    void resolve_court_stay(int64_t parcel_id, const std::string& vacate_order_ref);

    /**
     * @brief Resolve title mismatch between RoR owner and field claimant.
     */
    void resolve_title_mismatch(int64_t parcel_id, const std::string& field_claimant_name);

    /**
     * @brief Harmonize cadastral survey area through Joint Measurement Survey (JMS).
     */
    void harmonize_cadastral_survey(int64_t parcel_id, double verified_cadastral_sqm);

    /**
     * @brief Compute corridor-wide PCI and bottleneck unlock rankings.
     */
    PCIReport evaluate_corridor_pci(double total_corridor_length_km) const;
    std::vector<UnlockRanking> identify_unlock_priorities(double total_corridor_length_km, size_t top_k = 5) const;

    /**
     * @brief Query current state of a parcel.
     */
    models::ParcelState get_parcel_state(int64_t parcel_id) const;
    models::ParcelVersion get_parcel(int64_t parcel_id) const;

private:
    std::shared_ptr<core::BitemporalEventStore> event_store_;
    std::shared_ptr<ContradictionEngine> contradiction_engine_;
    std::shared_ptr<SIAInclusionEngine> sia_engine_;
    std::shared_ptr<PaymentReconciler> payment_reconciler_;
    std::shared_ptr<PCIEngine> pci_engine_;

    struct ParcelEntry {
        models::ParcelVersion parcel;
        models::ParcelState state{models::ParcelState::CANDIDATE};
        double chainage_start{0.0};
        double chainage_end{0.0};
        bool field_memo_verified{false};
        std::string memo_reference;
    };

    std::unordered_map<int64_t, ParcelEntry> parcels_;
};

} // namespace services
} // namespace dharti

#endif // DHARTI_SERVICES_WORKFLOW_COORDINATOR_HPP
