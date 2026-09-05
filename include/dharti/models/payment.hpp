#ifndef DHARTI_MODELS_PAYMENT_HPP
#define DHARTI_MODELS_PAYMENT_HPP

#include <string>

namespace dharti {
namespace models {

enum class PaymentState {
    OBLIGATION_CREATED = 0,
    SANCTIONED = 1,
    PFMS_INSTRUCTED = 2,
    BANK_ACKNOWLEDGED = 3,
    RECEIPT_CONFIRMED = 4,
    FAILED = 5
};

inline const char* payment_state_to_string(PaymentState s) {
    switch (s) {
        case PaymentState::OBLIGATION_CREATED: return "ObligationCreated";
        case PaymentState::SANCTIONED: return "Sanctioned";
        case PaymentState::PFMS_INSTRUCTED: return "PFMSInstructed";
        case PaymentState::BANK_ACKNOWLEDGED: return "BankAcknowledged";
        case PaymentState::RECEIPT_CONFIRMED: return "ReceiptConfirmed";
        case PaymentState::FAILED: return "Failed";
        default: return "Unknown";
    }
}

struct PaymentRecord {
    std::string payment_id;
    int parcel_id;
    std::string beneficiary_id;
    double amount_inr;
    PaymentState state;
    std::string sanction_order;
    std::string pfms_reference;
    bool bank_ack_received;
    bool receipt_or_court_deposit_confirmed;
    std::string failure_reason;
};

} // namespace models
} // namespace dharti

#endif // DHARTI_MODELS_PAYMENT_HPP
