#ifndef DHARTI_SERVICES_PAYMENT_RECONCILER_HPP
#define DHARTI_SERVICES_PAYMENT_RECONCILER_HPP

#include "dharti/models/payment.hpp"
#include "dharti/adapters/finance_adapter.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace dharti {
namespace services {

/**
 * @brief Summary report for parcel financial reconciliation.
 */
struct ParcelPaymentSummary {
    int64_t parcel_id{0};
    double total_obligation_inr{0.0};
    double total_disbursed_inr{0.0};
    double outstanding_balance_inr{0.0};
    bool is_fully_compensated{false};
    bool has_failed_transactions{false};
    std::vector<std::string> pending_reasons;
};

/**
 * @brief Payment Reconciliation Service.
 * 
 * Reconciles statutory financial obligations against PFMS bank settlements.
 * Enforces Invariant 3: No physical possession or construction readiness 
 * without verified receipt confirmation.
 */
class PaymentReconciler {
public:
    PaymentReconciler() = default;
    ~PaymentReconciler() = default;

    /**
     * @brief Register an approved statutory compensation obligation.
     */
    void register_obligation(const models::PaymentRecord& obligation);

    /**
     * @brief Process and reconcile an ingested PFMS bank settlement record.
     * 
     * @param advice Normalized payment record from FinanceAdapter.
     * @return true if reconciled successfully; false if error or mismatch.
     */
    bool reconcile_advice(const adapters::NormalizedPaymentRecord& advice);

    /**
     * @brief Retrieve detailed payment status for a specific parcel.
     */
    ParcelPaymentSummary get_parcel_summary(int64_t parcel_id) const;

    /**
     * @brief Checks if a parcel is 100% financially cleared.
     */
    bool is_parcel_financially_cleared(int64_t parcel_id) const;

    size_t total_obligations() const { return obligations_.size(); }
    size_t confirmed_receipts() const { return settled_count_; }
    size_t failed_reconciliations() const { return failed_count_; }

private:
    std::unordered_map<std::string, models::PaymentRecord> obligations_; // Keyed by payment_id / mandate_id
    size_t settled_count_{0};
    size_t failed_count_{0};
};

} // namespace services
} // namespace dharti

#endif // DHARTI_SERVICES_PAYMENT_RECONCILER_HPP
