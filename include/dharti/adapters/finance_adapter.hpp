#ifndef DHARTI_ADAPTERS_FINANCE_ADAPTER_HPP
#define DHARTI_ADAPTERS_FINANCE_ADAPTER_HPP

#include "dharti/adapters/adapter_interface.hpp"
#include <string>
#include <vector>
#include <optional>

namespace dharti {
namespace adapters {

/**
 * @brief Raw Payment Instruction and Bank Credit Advice from PFMS / State Treasury.
 */
struct RawPFMSAdvice {
    std::string sanction_order_no;
    std::string payment_mandate_id;
    int64_t parcel_id{0};
    std::string claimant_token;    // Salted tokenized identifier (INV-6: Public Redaction)
    double gross_amount_inr{0.0};
    double solatium_amount_inr{0.0};
    double interest_amount_inr{0.0};
    double net_payable_inr{0.0};
    std::string bank_utr_reference;
    std::string credit_status;     // "SUCCESS", "REJECTED", "PENDING"
    std::string error_code;        // e.g. "ERR_ACCOUNT_FROZEN", "ERR_INVALID_IFSC"
    std::string settlement_date;
};

/**
 * @brief Normalized financial settlement record.
 */
struct NormalizedPaymentRecord {
    std::string mandate_id;
    int64_t parcel_id{0};
    std::string claimant_token;
    double net_amount_inr{0.0};
    std::string bank_utr;
    bool is_settled{false};
    bool is_failed{false};
    std::string rejection_reason;
    std::string settlement_timestamp;
    IngestionMetadata metadata;
};

/**
 * @brief Federated Adapter for Public Financial Management System (PFMS) & Treasury.
 */
class FinanceAdapter : public IAdapter {
public:
    explicit FinanceAdapter(std::string treasury_code = "PFMS_CENTRAL");
    ~FinanceAdapter() override = default;

    std::string adapter_name() const override;
    std::string source_system_code() const override;
    std::string supported_schema_version() const override;

    /**
     * @brief Ingest and harmonize a PFMS payment advice.
     * 
     * @param advice Raw PFMS / Bank settlement advice.
     * @return NormalizedPaymentRecord with verified credit status or error classification.
     */
    NormalizedPaymentRecord ingest_payment_advice(const RawPFMSAdvice& advice);

    size_t successful_disbursements() const { return success_count_; }
    size_t failed_disbursements() const { return failed_count_; }

private:
    std::string treasury_code_;
    size_t success_count_{0};
    size_t failed_count_{0};
};

} // namespace adapters
} // namespace dharti

#endif // DHARTI_ADAPTERS_FINANCE_ADAPTER_HPP
