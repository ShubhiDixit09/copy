/**
 * DHARTI - Interactive Manual Testing Sandbox Module
 * Allows operators and auditors to execute state transitions, dispute resolutions, and invariant defenses.
 */

class TestSandbox {
  constructor() {
    this.currentStep = 1;
  }

  runMultiAgencyCheck() {
    const p118 = window.dataStore.parcels.find(p => p.parcel_id === 118);
    const contradictions = [
      {
        code: "ERR_AREA_DISCREPANCY",
        parcel_id: 118,
        message: `Bhoomi Cadastral Area (17,250 sqm) exceeds RoR Extent (15,000 sqm) by 15.0% (> 1% statutory threshold).`
      },
      {
        code: "ERR_JUDICIAL_INJUNCTION",
        parcel_id: 118,
        message: `High Court of Karnataka WP 4021/2023 stay order in force (CNR: KABC010040212023).`
      },
      {
        code: "ERR_PFMS_REJECTION",
        parcel_id: 118,
        message: `PFMS DBT mandate rejected: Bank Account frozen by judicial order (AC_FROZEN_BY_AUTHORITY).`
      }
    ];

    window.dataStore.appendAudit("MultiAgencyContradictionCheck", "Detected 3 legal/financial exceptions on Parcel P-118");
    return {
      status: "CONTRADICTIONS_DETECTED",
      contradictions: contradictions
    };
  }

  attemptPrematureHandover(parcelId = 118) {
    const parcel = window.dataStore.parcels.find(p => p.parcel_id === parcelId);
    if (!parcel) return { success: false, reason: "Parcel not found" };

    // Invariant 2 Check: Judicial Injunction
    if (parcel.court_stay) {
      window.dataStore.appendAudit("InvariantViolationBlocked", `Attempted handover of P-${parcelId} blocked: Invariant 2 (Active Court Injunction)`);
      return {
        success: false,
        invariant: "INVARIANT-2",
        title: "INVARIANT-2 VIOLATION: Active Judicial Injunction",
        reason: `Parcel P-${parcelId} is subject to High Court of Karnataka stay in WP 4021/2023. Construction handover is legally prohibited.`
      };
    }

    // Invariant 3 Check: Financial Compensation Not Disbursed
    if (parcel.disbursed_amount_inr < parcel.sanctioned_amount_inr || !parcel.bank_utr) {
      window.dataStore.appendAudit("InvariantViolationBlocked", `Attempted handover of P-${parcelId} blocked: Invariant 3 (Compensation Not Reconciled)`);
      return {
        success: false,
        invariant: "INVARIANT-3",
        title: "INVARIANT-3 VIOLATION: Financial Compensation Unsettled",
        reason: `Statutory compensation of ₹${(parcel.sanctioned_amount_inr / 1e7).toFixed(2)} Cr has not been settled in beneficiary bank account (Status: ${parcel.payment_status}).`
      };
    }

    parcel.state = "ConstructionReady";
    return { success: true, message: `Parcel P-${parcelId} handover approved.` };
  }

  runSIAInclusionAudit() {
    const missing = window.dataStore.households.filter(h => h.vulnerable && !h.award);
    const compliant = missing.length === 0;

    window.dataStore.appendAudit("SIAInclusionAuditExecuted", `Audited 10 households; ${missing.length} unmapped vulnerable families detected.`);

    return {
      totalHouseholds: window.dataStore.households.length,
      mappedCount: window.dataStore.households.filter(h => h.award).length,
      missingVulnerable: missing,
      circuitBreakerEngaged: !compliant,
      ruling: compliant ? "COMPLIANT" : "BLOCKED_BY_INVARIANT_4"
    };
  }

  vacateCourtStay(parcelId = 118) {
    const parcel = window.dataStore.parcels.find(p => p.parcel_id === parcelId);
    if (!parcel) return;

    parcel.court_stay = false;
    parcel.stay_details = "Vacated under High Court IA-3/2026; compensation deposited under Section 3H(4)";
    if (parcel.state === "InjunctionImposed") {
      parcel.state = "AwardDeclared";
    }

    window.dataStore.appendAudit("CourtInjunctionVacated", `High Court WP 4021/2023 stay vacated for P-${parcelId}`);
    return { success: true, parcel: parcel };
  }

  reconcilePFMSPayment(parcelId = 118) {
    const parcel = window.dataStore.parcels.find(p => p.parcel_id === parcelId);
    if (!parcel) return;

    parcel.disbursed_amount_inr = parcel.sanctioned_amount_inr;
    parcel.bank_utr = "SBIN426245890118";
    parcel.payment_status = "SUCCESS";
    if (parcel.state === "AwardDeclared") {
      parcel.state = "PossessionConfirmed";
    }

    window.dataStore.appendAudit("PFMSPaymentReconciled", `Disbursed ₹4.50 Cr to P-${parcelId} under Bank UTR SBIN426245890118`);
    return { success: true, parcel: parcel };
  }

  confirmFieldPossession(parcelId = 118) {
    const parcel = window.dataStore.parcels.find(p => p.parcel_id === parcelId);
    if (!parcel) return;

    parcel.state = "ConstructionReady";
    window.dataStore.appendAudit("FieldPossessionCertified", `Geotagged Panchnama certified for P-${parcelId}; Parcel is ConstructionReady`);
    return { success: true, parcel: parcel };
  }

  resolveMissingFamilies() {
    window.dataStore.households.forEach(h => {
      if (h.vulnerable && !h.award) {
        h.award = true;
        h.rnr = true;
      }
    });
    window.dataStore.appendAudit("SIAHouseholdsMapped", "Supplementary Section 23 award published mapping 4 vulnerable families to R&R layout");
    return { success: true };
  }
}

window.TestSandbox = TestSandbox;
