#ifndef DHARTI_MODELS_PARCEL_HPP
#define DHARTI_MODELS_PARCEL_HPP

#include <string>

namespace dharti {
namespace models {

enum class ParcelState {
    CANDIDATE = 0,
    NOTIFIED = 1,
    AWARDED = 2,
    POSSESSION_VERIFIED = 3,
    CONSTRUCTION_READY = 4
};

inline const char* parcel_state_to_string(ParcelState s) {
    switch (s) {
        case ParcelState::CANDIDATE: return "Candidate";
        case ParcelState::NOTIFIED: return "Notified";
        case ParcelState::AWARDED: return "Awarded";
        case ParcelState::POSSESSION_VERIFIED: return "PossessionVerified";
        case ParcelState::CONSTRUCTION_READY: return "ConstructionReady";
        default: return "Unknown";
    }
}

struct ParcelVersion {
    int parcel_id;
    std::string ulpin;
    std::string survey_number;
    ParcelState state;
    double start_chainage;
    double end_chainage;
    double ror_area_sqm;
    double cadastral_area_sqm;
    std::string ror_owner;
    std::string field_claimant;
    bool has_active_stay;
    bool is_possession_witnessed;
};

} // namespace models
} // namespace dharti

#endif // DHARTI_MODELS_PARCEL_HPP
