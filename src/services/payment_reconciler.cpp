#include "dharti/services/payment_reconciler.hpp"
#include <cmath>

namespace dharti {
namespace services {

void PaymentReconciler::register_obligation(const models::PaymentRecord& obligation) {
    obligations_[obligation.payment_id] = obligation;
}

bool PaymentReconciler::reconcile_advice(const adapters::NormalizedPaymentRecord& advice) {
    auto it = obligations_.find(advice.mandate_id);
    if (it == obligations_.end()) {
        // Look up by parcel_id if mandate not directly keyed
        for (auto& pair : obligations_) {
            if (pair.second.parcel_id == advice.parcel_id) {
                it = obligations_.find(pair.first);
                break;
            }
        }
    }

    if (it == obligations_.end()) {
        failed_count_++;
        return false; // Unknown payment reference
    }

    models::PaymentRecord& rec = it->second;
    if (advice.is_settled) {
        rec.state = models::PaymentState::RECEIPT_CONFIRMED;
        rec.receipt_or_court_deposit_confirmed = true;
        rec.bank_ack_received = true;
        rec.bank_utr = advice.bank_utr;
        rec.disbursed_amount = advice.net_amount_inr;
        settled_count_++;
        return true;
    } else if (advice.is_failed) {
        rec.state = models::PaymentState::FAILED;
        rec.receipt_or_court_deposit_confirmed = false;
        rec.failure_reason = advice.rejection_reason;
        failed_count_++;
        return false;
    }

    return false;
}

ParcelPaymentSummary PaymentReconciler::get_parcel_summary(int64_t parcel_id) const {
    ParcelPaymentSummary summary;
    summary.parcel_id = parcel_id;

    bool has_obligations = false;
    for (const auto& pair : obligations_) {
        const auto& rec = pair.second;
        if (rec.parcel_id == parcel_id) {
            has_obligations = true;
            summary.total_obligation_inr += rec.amount_inr;
            if (rec.state == models::PaymentState::RECEIPT_CONFIRMED) {
                summary.total_disbursed_inr += rec.disbursed_amount;
            } else if (rec.state == models::PaymentState::FAILED) {
                summary.has_failed_transactions = true;
                summary.pending_reasons.push_back("Payment failed: " + rec.failure_reason);
            } else {
                summary.pending_reasons.push_back("Payment pending confirmation");
            }
        }
    }

    summary.outstanding_balance_inr = summary.total_obligation_inr - summary.total_disbursed_inr;
    if (summary.outstanding_balance_inr < 0.0) {
        summary.outstanding_balance_inr = 0.0;
    }

    summary.is_fully_compensated = has_obligations &&
                                   (summary.outstanding_balance_inr <= 0.01) &&
                                   !summary.has_failed_transactions;

    return summary;
}

bool PaymentReconciler::is_parcel_financially_cleared(int64_t parcel_id) const {
    return get_parcel_summary(parcel_id).is_fully_compensated;
}

} // namespace services
} // namespace dharti
