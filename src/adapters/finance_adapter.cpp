#include "dharti/adapters/finance_adapter.hpp"
#include "dharti/utils/json.hpp"
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

std::vector<NormalizedPaymentRecord> FinanceAdapter::ingest_from_file(const std::string& pfms_file_path) {
    std::vector<NormalizedPaymentRecord> results;
    utils::JsonValue root = utils::JsonValue::parse_file(pfms_file_path);

    const auto& advices = root["advices"];
    for (size_t i = 0; i < advices.size(); ++i) {
        const auto& a = advices[i];
        RawPFMSAdvice raw;
        raw.sanction_order_no = a["sanction_order_no"].as_string();
        raw.payment_mandate_id = a["payment_mandate_id"].as_string();
        raw.parcel_id = a["parcel_id"].as_int();
        raw.claimant_token = a["claimant_token"].as_string();
        raw.gross_amount_inr = a["gross_amount_inr"].as_double();
        raw.solatium_amount_inr = a["solatium_amount_inr"].as_double();
        raw.interest_amount_inr = a["interest_amount_inr"].as_double();
        raw.net_payable_inr = a["net_payable_inr"].as_double();
        raw.bank_utr_reference = a["bank_utr_reference"].as_string();
        raw.credit_status = a["credit_status"].as_string();
        raw.error_code = a["error_code"].as_string();
        raw.settlement_date = a["settlement_date"].as_string();

        results.push_back(ingest_payment_advice(raw));
    }

    return results;
}

} // namespace adapters
} // namespace dharti

