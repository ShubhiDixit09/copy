#ifndef DHARTI_C_API_H
#define DHARTI_C_API_H

#include "pci_engine.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
  #define DHARTI_EXPORT __declspec(dllexport)
#else
  #define DHARTI_EXPORT __attribute__((visibility("default")))
#endif

typedef struct {
    int parcel_id;
    double ror_area_sqm;
    double cadastral_area_sqm;
    const char* ror_owner;
    const char* field_claimant;
    int has_active_stay; // 1 = yes, 0 = no
} CParcelInput;

typedef struct {
    int parcel_id;
    int type_code;       // 0: TitleMismatch, 1: AreaDiscrepancy, 2: ActiveCourtStay
    int severity_code;   // 0: Critical, 1: Warning, 2: Advisory
    char case_id[64];
    char description[256];
    char blocking_gate[64];
    char assigned_owner[64];
    int sla_hours;
} CContradictionCase;

typedef struct {
    const char* household_id;
    const char* category;
    int family_member_count;
    int is_vulnerable;
    int has_award_mapping;
    int has_rnr_mapping;
    int is_reviewed_ineligible;
    const char* speaking_order_ref;
} CHouseholdInput;

typedef struct {
    int total_observed;
    int award_mapped;
    int rnr_mapped;
    int lawfully_excluded;
    int missing_unaccounted;
    int is_compliant;
} CSIAAuditSummary;

/**
 * Evaluates contradictions on a single parcel using C++ Contradiction Engine.
 */
DHARTI_EXPORT int evaluate_parcel_contradictions_native(
    const CParcelInput* parcel,
    double area_tolerance_percent,
    CContradictionCase* out_cases,
    int max_cases,
    int* actual_cases_count
);

/**
 * Audits SIA household universe using C++ SIA Inclusion Engine.
 */
DHARTI_EXPORT int audit_sia_inclusion_native(
    const CHouseholdInput* households,
    int count,
    CSIAAuditSummary* out_summary
);

#ifdef __cplusplus
}
#endif

#endif // DHARTI_C_API_H
