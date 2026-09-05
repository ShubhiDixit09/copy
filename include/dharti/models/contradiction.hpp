#ifndef DHARTI_MODELS_CONTRADICTION_HPP
#define DHARTI_MODELS_CONTRADICTION_HPP

#include <string>

namespace dharti {
namespace models {

enum class ContradictionSeverity {
    CRITICAL = 0, // Blocks downstream statutory gate
    WARNING = 1,  // Requires verification within SLA
    ADVISORY = 2  // Noted for audit trace
};

enum class ContradictionType {
    TITLE_MISMATCH = 0,
    AREA_DISCREPANCY = 1,
    ACTIVE_COURT_STAY = 2,
    UNMATCHED_SIA_HOUSEHOLD = 3,
    PAYMENT_FAILED_RECONCILIATION = 4
};

inline const char* contradiction_severity_to_string(ContradictionSeverity s) {
    switch (s) {
        case ContradictionSeverity::CRITICAL: return "CRITICAL";
        case ContradictionSeverity::WARNING: return "WARNING";
        case ContradictionSeverity::ADVISORY: return "ADVISORY";
        default: return "UNKNOWN";
    }
}

inline const char* contradiction_type_to_string(ContradictionType t) {
    switch (t) {
        case ContradictionType::TITLE_MISMATCH: return "TitleMismatch";
        case ContradictionType::AREA_DISCREPANCY: return "AreaDiscrepancy";
        case ContradictionType::ACTIVE_COURT_STAY: return "ActiveCourtStay";
        case ContradictionType::UNMATCHED_SIA_HOUSEHOLD: return "UnmatchedSIAHousehold";
        case ContradictionType::PAYMENT_FAILED_RECONCILIATION: return "PaymentFailedReconciliation";
        default: return "Unknown";
    }
}

struct ContradictionCase {
    std::string case_id;
    int parcel_id;
    ContradictionType type;
    ContradictionSeverity severity;
    std::string description;
    std::string blocking_gate; // e.g. "AwardGate", "PossessionGate", "ConstructionGate"
    std::string assigned_owner_role; // "LAA", "Collector", "RevenueInspector", "AccountsOfficer"
    int sla_hours;
    bool is_resolved;
    std::string resolution_order;
};

} // namespace models
} // namespace dharti

#endif // DHARTI_MODELS_CONTRADICTION_HPP
