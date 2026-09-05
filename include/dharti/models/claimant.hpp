#ifndef DHARTI_MODELS_CLAIMANT_HPP
#define DHARTI_MODELS_CLAIMANT_HPP

#include <string>

namespace dharti {
namespace models {

enum class ClaimantState {
    OBSERVED_IN_SIA = 0,
    IDENTITY_RESOLVED = 1,
    INTEREST_VERIFIED = 2,
    ENTITLEMENT_MAPPED = 3,
    RESOLVED = 4
};

inline const char* claimant_state_to_string(ClaimantState s) {
    switch (s) {
        case ClaimantState::OBSERVED_IN_SIA: return "ObservedInSIA";
        case ClaimantState::IDENTITY_RESOLVED: return "IdentityResolved";
        case ClaimantState::INTEREST_VERIFIED: return "InterestVerified";
        case ClaimantState::ENTITLEMENT_MAPPED: return "EntitlementMapped";
        case ClaimantState::RESOLVED: return "Resolved";
        default: return "Unknown";
    }
}

struct Household {
    std::string household_id;
    std::string head_token;
    int family_member_count;
    bool is_vulnerable;           // Landless, SC/ST, BPL, single-woman head
    std::string category;         // "AgriculturalTenant", "LivelihoodDependent", "TitleHolder"
    bool has_award_mapping;       // Linked to financial award schedule
    bool has_rnr_mapping;         // Linked to R&R rehabilitation package
    bool is_reviewed_ineligible;  // Valid speaking order documenting exclusion
    std::string speaking_order_ref;
};

} // namespace models
} // namespace dharti

#endif // DHARTI_MODELS_CLAIMANT_HPP
