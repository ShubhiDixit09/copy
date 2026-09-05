#include "dharti/adapters/finance_adapter.hpp"
#include <sstream>

namespace dharti {
namespace adapters {

FinanceAdapter::FinanceAdapter(std::string treasury_code)
    : treasury_code_(std::move(treasury_code)) {}

std::string FinanceAdapter::adapter_name() const {
    return "PFMSFederatedAdapter-" + treasury_code_;
}

std::string FinanceAdapter::source_system_code() const {
    return treasury_code_;
}

std::string FinanceAdapter::supported_schema_version() const {
    return "v3.2-PFMS-DirectDisbursement";
}

NormalizedPaymentRecord FinanceAdapter::ingest_payment_advice(const RawPFMSAdvice& advice) {
    NormalizedPaymentRecord record;
    record.mandate_id = advice.payment_mandate_id;
    record.parcel_id = advice.parcel_id;
    record.claimant_token = advice.claimant_token;
    record.net_amount_inr = advice.net_payable_inr;
    record.bank_utr = advice.bank_utr_reference;
    record.settlement_timestamp = advice.settlement_date;

    if (advice.credit_status == "SUCCESS") {
        record.is_settled = true;
        record.is_failed = false;
        success_count_++;
    } else if (advice.credit_status == "REJECTED") {
        record.is_settled = false;
        record.is_failed = true;
        record.rejection_reason = advice.error_code.empty() ? "UNKNOWN_BANK_REJECTION" : advice.error_code;
        failed_count_++;
    } else {
        record.is_settled = false;
        record.is_failed = false;
        record.rejection_reason = "PAYMENT_PENDING_SETTLEMENT";
    }

    std::stringstream raw_builder;
    raw_builder << advice.payment_mandate_id << "|"
                << advice.parcel_id << "|"
                << advice.claimant_token << "|"
                << advice.net_payable_inr << "|"
                << advice.bank_utr_reference << "|"
                << advice.credit_status;

    record.metadata.source_system = source_system_code();
    record.metadata.source_record_id = advice.payment_mandate_id;
    record.metadata.schema_version = supported_schema_version();
    record.metadata.payload_checksum = compute_hash(raw_builder.str());
    record.metadata.ingestion_timestamp = current_utc_timestamp();

    if (advice.net_payable_inr <= 0.0 || advice.payment_mandate_id.empty()) {
        record.metadata.is_quarantined = true;
        record.metadata.quarantine_reason = "Invalid net payable amount or missing mandate ID.";
    } else {
        record.metadata.is_quarantined = false;
    }

    return record;
}

} // namespace adapters
} // namespace dharti
