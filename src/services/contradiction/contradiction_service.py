"""Cross-source contradiction detection service.

Evaluates RoR records, cadastral maps, and court stay orders using C++ acceleration with Python fallback.
"""

import ctypes
import os
from dataclasses import dataclass
from typing import Any, Dict, List, Optional
from src.core.base import BaseService
from src.utils.logger import get_logger

logger = get_logger("dharti.services.contradiction")


class _CParcelInput(ctypes.Structure):
    _fields_ = [
        ("parcel_id", ctypes.c_int),
        ("ror_area_sqm", ctypes.c_double),
        ("cadastral_area_sqm", ctypes.c_double),
        ("ror_owner", ctypes.c_char_p),
        ("field_claimant", ctypes.c_char_p),
        ("has_active_stay", ctypes.c_int),
    ]


class _CContradictionCase(ctypes.Structure):
    _fields_ = [
        ("parcel_id", ctypes.c_int),
        ("type_code", ctypes.c_int),
        ("severity_code", ctypes.c_int),
        ("case_id", ctypes.c_char * 64),
        ("description", ctypes.c_char * 256),
        ("blocking_gate", ctypes.c_char * 64),
        ("assigned_owner", ctypes.c_char * 64),
        ("sla_hours", ctypes.c_int),
    ]


@dataclass
class ContradictionCaseDTO:
    case_id: str
    parcel_id: int
    type_name: str
    severity: str
    description: str
    blocking_gate: str
    assigned_owner: str
    sla_hours: int

    def to_dict(self) -> Dict[str, Any]:
        return {
            "case_id": self.case_id,
            "parcel_id": self.parcel_id,
            "type": self.type_name,
            "severity": self.severity,
            "description": self.description,
            "blocking_gate": self.blocking_gate,
            "assigned_owner": self.assigned_owner,
            "sla_hours": self.sla_hours,
        }


class ContradictionService(BaseService):
    """Detects deterministic discrepancies across RoR, cadastral maps, and court registries."""

    def __init__(self, area_tolerance_percent: float = 1.0, force_python: bool = False):
        self.area_tolerance_percent = area_tolerance_percent
        self.force_python = force_python
        self._native_lib = None
        if not force_python:
            self._native_lib = self._load_native_lib()

    def health_check(self) -> Dict[str, Any]:
        return {
            "service": "ContradictionService",
            "status": "healthy",
            "is_native_accelerated": self._native_lib is not None,
            "area_tolerance_percent": self.area_tolerance_percent,
        }

    def _load_native_lib(self) -> Optional[ctypes.CDLL]:
        base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../native"))
        core_name = "dharti_core.dll" if os.name == "nt" else "libdharti_core.so"
        core_path = os.path.join(base_dir, core_name)
        if os.path.exists(core_path):
            try:
                lib = ctypes.CDLL(core_path)
                lib.evaluate_parcel_contradictions_native.argtypes = [
                    ctypes.POINTER(_CParcelInput),
                    ctypes.c_double,
                    ctypes.POINTER(_CContradictionCase),
                    ctypes.c_int,
                    ctypes.POINTER(ctypes.c_int),
                ]
                lib.evaluate_parcel_contradictions_native.restype = ctypes.c_int
                return lib
            except Exception as e:
                logger.warning("Could not bind native contradiction API: %s", e)
        return None

    def evaluate_parcel(self, parcel: Dict[str, Any]) -> List[ContradictionCaseDTO]:
        if self._native_lib:
            return self._evaluate_native(parcel)
        return self._evaluate_python(parcel)

    def _evaluate_native(self, parcel: Dict[str, Any]) -> List[ContradictionCaseDTO]:
        c_in = _CParcelInput()
        c_in.parcel_id = int(parcel.get("parcel_id", 0))
        c_in.ror_area_sqm = float(parcel.get("ror_area_sqm", 0.0))
        c_in.cadastral_area_sqm = float(parcel.get("cadastral_area_sqm", 0.0))
        c_in.ror_owner = parcel.get("ror_owner", "").encode("utf-8")
        c_in.field_claimant = parcel.get("field_claimant", "").encode("utf-8")
        c_in.has_active_stay = 1 if parcel.get("has_active_stay", False) else 0

        max_cases = 10
        c_cases = (_CContradictionCase * max_cases)()
        actual_count = ctypes.c_int(0)

        code = self._native_lib.evaluate_parcel_contradictions_native(
            ctypes.byref(c_in),
            ctypes.c_double(self.area_tolerance_percent),
            c_cases,
            ctypes.c_int(max_cases),
            ctypes.byref(actual_count),
        )
        if code != 0:
            raise RuntimeError(f"Native evaluate_parcel_contradictions failed with code {code}")

        severity_map = {0: "CRITICAL", 1: "WARNING", 2: "ADVISORY"}
        type_map = {
            0: "TitleMismatch",
            1: "AreaDiscrepancy",
            2: "ActiveCourtStay",
        }

        results = []
        for i in range(actual_count.value):
            item = c_cases[i]
            results.append(
                ContradictionCaseDTO(
                    case_id=item.case_id.decode("utf-8"),
                    parcel_id=item.parcel_id,
                    type_name=type_map.get(item.type_code, "Unknown"),
                    severity=severity_map.get(item.severity_code, "WARNING"),
                    description=item.description.decode("utf-8"),
                    blocking_gate=item.blocking_gate.decode("utf-8"),
                    assigned_owner=item.assigned_owner.decode("utf-8"),
                    sla_hours=item.sla_hours,
                )
            )
        return results

    def _evaluate_python(self, parcel: Dict[str, Any]) -> List[ContradictionCaseDTO]:
        pid = parcel.get("parcel_id", 0)
        cases = []

        # Stay check
        if parcel.get("has_active_stay", False):
            cases.append(
                ContradictionCaseDTO(
                    case_id=f"EX-STAY-{pid}",
                    parcel_id=pid,
                    type_name="ActiveCourtStay",
                    severity="CRITICAL",
                    description="Active stay order / status quo injunction recorded in eCourts/RCCMS registry.",
                    blocking_gate="PossessionAndConstructionGate",
                    assigned_owner="LegalOfficer",
                    sla_hours=48,
                )
            )

        # Title mismatch
        ror_owner = parcel.get("ror_owner", "").strip()
        field_claimant = parcel.get("field_claimant", "").strip()
        if ror_owner and field_claimant and ror_owner != field_claimant:
            cases.append(
                ContradictionCaseDTO(
                    case_id=f"EX-TITLE-{pid}",
                    parcel_id=pid,
                    type_name="TitleMismatch",
                    severity="CRITICAL",
                    description=f"Registered RoR titleholder ({ror_owner}) contradicts on-ground surveyed claimant ({field_claimant}).",
                    blocking_gate="AwardGate",
                    assigned_owner="CompetentAuthorityLAA",
                    sla_hours=72,
                )
            )

        # Area discrepancy
        ror_area = float(parcel.get("ror_area_sqm", 0.0))
        cad_area = float(parcel.get("cadastral_area_sqm", 0.0))
        if ror_area > 0 and cad_area > 0:
            diff = abs(cad_area - ror_area)
            pct = (diff / ror_area) * 100.0
            if pct > self.area_tolerance_percent:
                sev = "CRITICAL" if pct > 5.0 else "WARNING"
                cases.append(
                    ContradictionCaseDTO(
                        case_id=f"EX-AREA-{pid}",
                        parcel_id=pid,
                        type_name="AreaDiscrepancy",
                        severity=sev,
                        description=f"Cadastral vector area ({cad_area} sqm) differs from recorded RoR area ({ror_area} sqm) by {pct:.2f}%.",
                        blocking_gate="AwardGate" if sev == "CRITICAL" else "NoticeGate",
                        assigned_owner="DistrictSurveyOfficer",
                        sla_hours=96,
                    )
                )

        return cases
