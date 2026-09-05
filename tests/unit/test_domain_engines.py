"""Unit tests for Contradiction Engine and SIA Inclusion Engine (C++ and Python parity)."""

import unittest
from src.services.contradiction.contradiction_service import ContradictionService
from src.services.sia.sia_service import SIAInclusionService

class TestDomainEngines(unittest.TestCase):
    """Verifies contradiction detection and SIA inclusion audit behavior."""

    def test_contradiction_detection_clean_parcel(self):
        service = ContradictionService(area_tolerance_percent=1.0)
        clean_parcel = {
            "parcel_id": 101,
            "ror_area_sqm": 1000.0,
            "cadastral_area_sqm": 1005.0, # 0.5% diff <= 1.0%
            "ror_owner": "Ramesh Kumar",
            "field_claimant": "Ramesh Kumar",
            "has_active_stay": False,
        }
        cases = service.evaluate_parcel(clean_parcel)
        self.assertEqual(len(cases), 0)

    def test_contradiction_detection_conflicted_parcel(self):
        # Parcel P-118 has court stay, title mismatch, and area discrepancy
        py_service = ContradictionService(force_python=True)
        native_service = ContradictionService(force_python=False)

        conflicted_parcel = {
            "parcel_id": 118,
            "ror_area_sqm": 1000.0,
            "cadastral_area_sqm": 1150.0, # 15% discrepancy
            "ror_owner": "Ramesh Kumar",
            "field_claimant": "Suresh Kumar",
            "has_active_stay": True,
        }

        py_cases = py_service.evaluate_parcel(conflicted_parcel)
        self.assertEqual(len(py_cases), 3)
        types = {c.type_name for c in py_cases}
        self.assertIn("ActiveCourtStay", types)
        self.assertIn("TitleMismatch", types)
        self.assertIn("AreaDiscrepancy", types)

        if native_service._native_lib:
            native_cases = native_service.evaluate_parcel(conflicted_parcel)
            self.assertEqual(len(native_cases), 3)
            native_types = {c.type_name for c in native_cases}
            self.assertEqual(types, native_types)

    def test_sia_inclusion_audit(self):
        py_service = SIAInclusionService(force_python=True)
        native_service = SIAInclusionService(force_python=False)

        universe = []
        # 6 compliant households
        for i in range(1, 7):
            universe.append({
                "household_id": f"HH-{i}",
                "category": "TitleHolder",
                "family_member_count": 4,
                "is_vulnerable": False,
                "has_award_mapping": True,
                "has_rnr_mapping": False,
                "is_reviewed_ineligible": False,
            })
        # 4 missing vulnerable households (from SIH demo walkthrough)
        for i in range(7, 11):
            universe.append({
                "household_id": f"HH-{i}",
                "category": "LivelihoodDependent",
                "family_member_count": 5,
                "is_vulnerable": True,
                "has_award_mapping": False,
                "has_rnr_mapping": False,
                "is_reviewed_ineligible": False,
            })

        py_report = py_service.audit_universe(universe)
        self.assertEqual(py_report.total_observed, 10)
        self.assertEqual(py_report.award_mapped, 6)
        self.assertEqual(py_report.missing_unaccounted, 4)
        self.assertFalse(py_report.is_compliant)

        if native_service._native_lib:
            native_report = native_service.audit_universe(universe)
            self.assertEqual(native_report.total_observed, 10)
            self.assertEqual(native_report.missing_unaccounted, 4)
            self.assertTrue(native_report.is_accelerated)

if __name__ == "__main__":
    unittest.main()
