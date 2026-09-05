"""SIA (Social Impact Assessment) Inclusion Service enforcing No Family Invisible rule.

Uses C++ acceleration via ctypes with pure Python fallback.
"""

import ctypes
import os
from dataclasses import dataclass
from typing import Any, Dict, List, Optional
from src.core.base import BaseService
from src.utils.logger import get_logger

logger = get_logger("dharti.services.sia")


class _CHouseholdInput(ctypes.Structure):
    _fields_ = [
        ("household_id", ctypes.c_char_p),
        ("category", ctypes.c_char_p),
        ("family_member_count", ctypes.c_int),
        ("is_vulnerable", ctypes.c_int),
        ("has_award_mapping", ctypes.c_int),
        ("has_rnr_mapping", ctypes.c_int),
        ("is_reviewed_ineligible", ctypes.c_int),
        ("speaking_order_ref", ctypes.c_char_p),
    ]


class _CSIAAuditSummary(ctypes.Structure):
    _fields_ = [
        ("total_observed", ctypes.c_int),
        ("award_mapped", ctypes.c_int),
        ("rnr_mapped", ctypes.c_int),
        ("lawfully_excluded", ctypes.c_int),
        ("missing_unaccounted", ctypes.c_int),
        ("is_compliant", ctypes.c_int),
    ]


@dataclass
class SIAAuditSummaryDTO:
    total_observed: int
    award_mapped: int
    rnr_mapped: int
    lawfully_excluded: int
    missing_unaccounted: int
    is_compliant: bool
    is_accelerated: bool

    def to_dict(self) -> Dict[str, Any]:
        return {
            "total_observed": self.total_observed,
            "award_mapped": self.award_mapped,
            "rnr_mapped": self.rnr_mapped,
            "lawfully_excluded": self.lawfully_excluded,
            "missing_unaccounted": self.missing_unaccounted,
            "is_compliant": self.is_compliant,
            "is_accelerated": self.is_accelerated,
        }


class SIAInclusionService(BaseService):
    """Enforces the 'No Family Invisible' rule from RFCTLARR / World Bank ESS5 standards."""

    def __init__(self, force_python: bool = False):
        self.force_python = force_python
        self._native_lib = None
        if not force_python:
            self._native_lib = self._load_native_lib()

    def health_check(self) -> Dict[str, Any]:
        return {
            "service": "SIAInclusionService",
            "status": "healthy",
            "is_native_accelerated": self._native_lib is not None,
        }

    def _load_native_lib(self) -> Optional[ctypes.CDLL]:
        base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../native"))
        core_name = "dharti_core.dll" if os.name == "nt" else "libdharti_core.so"
        core_path = os.path.join(base_dir, core_name)
        if os.path.exists(core_path):
            try:
                lib = ctypes.CDLL(core_path)
                lib.audit_sia_inclusion_native.argtypes = [
                    ctypes.POINTER(_CHouseholdInput),
                    ctypes.c_int,
                    ctypes.POINTER(_CSIAAuditSummary),
                ]
                lib.audit_sia_inclusion_native.restype = ctypes.c_int
                return lib
            except Exception as e:
                logger.warning("Could not bind native SIA inclusion API: %s", e)
        return None

    def audit_universe(self, households: List[Dict[str, Any]]) -> SIAAuditSummaryDTO:
        if self._native_lib:
            return self._audit_native(households)
        return self._audit_python(households)

    def _audit_native(self, households: List[Dict[str, Any]]) -> SIAAuditSummaryDTO:
        n = len(households)
        c_hh = (_CHouseholdInput * n)()
        for i, h in enumerate(households):
            c_hh[i].household_id = h.get("household_id", f"HH-{i}").encode("utf-8")
            c_hh[i].category = h.get("category", "General").encode("utf-8")
            c_hh[i].family_member_count = int(h.get("family_member_count", 1))
            c_hh[i].is_vulnerable = 1 if h.get("is_vulnerable", False) else 0
            c_hh[i].has_award_mapping = 1 if h.get("has_award_mapping", False) else 0
            c_hh[i].has_rnr_mapping = 1 if h.get("has_rnr_mapping", False) else 0
            c_hh[i].is_reviewed_ineligible = 1 if h.get("is_reviewed_ineligible", False) else 0
            c_hh[i].speaking_order_ref = h.get("speaking_order_ref", "").encode("utf-8")

        summary = _CSIAAuditSummary()
        code = self._native_lib.audit_sia_inclusion_native(c_hh, ctypes.c_int(n), ctypes.byref(summary))
        if code != 0:
            raise RuntimeError(f"Native audit_sia_inclusion failed with code {code}")

        return SIAAuditSummaryDTO(
            total_observed=summary.total_observed,
            award_mapped=summary.award_mapped,
            rnr_mapped=summary.rnr_mapped,
            lawfully_excluded=summary.lawfully_excluded,
            missing_unaccounted=summary.missing_unaccounted,
            is_compliant=(summary.is_compliant != 0),
            is_accelerated=True,
        )

    def _audit_python(self, households: List[Dict[str, Any]]) -> SIAAuditSummaryDTO:
        total = len(households)
        award_mapped = 0
        rnr_mapped = 0
        lawfully_excluded = 0
        missing = 0

        for h in households:
            has_award = bool(h.get("has_award_mapping", False))
            has_rnr = bool(h.get("has_rnr_mapping", False))
            is_ex = bool(h.get("is_reviewed_ineligible", False)) and bool(h.get("speaking_order_ref", ""))

            if has_award:
                award_mapped += 1
            if has_rnr:
                rnr_mapped += 1
            if is_ex:
                lawfully_excluded += 1

            if not (has_award or has_rnr or is_ex):
                missing += 1

        return SIAAuditSummaryDTO(
            total_observed=total,
            award_mapped=award_mapped,
            rnr_mapped=rnr_mapped,
            lawfully_excluded=lawfully_excluded,
            missing_unaccounted=missing,
            is_compliant=(missing == 0),
            is_accelerated=False,
        )
