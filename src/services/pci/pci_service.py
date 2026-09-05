"""Possession Continuity Index (PCI) and corridor unlock simulation service.

Supports native C++ acceleration via ctypes with seamless Python fallback.
"""

import ctypes
import os
import shutil
import subprocess
from dataclasses import dataclass
from typing import Any, Dict, List, Optional, Tuple
from src.utils.logger import get_logger

logger = get_logger("dharti.services.pci")


class _NativeChainageInterval(ctypes.Structure):
    _fields_ = [
        ("parcel_id", ctypes.c_int),
        ("start_chainage", ctypes.c_double),
        ("end_chainage", ctypes.c_double),
        ("is_ready", ctypes.c_int),
    ]


class _NativePCIResult(ctypes.Structure):
    _fields_ = [
        ("total_length", ctypes.c_double),
        ("total_ready_length", ctypes.c_double),
        ("total_blocked_length", ctypes.c_double),
        ("max_continuous_ready_length", ctypes.c_double),
        ("pci", ctypes.c_double),
    ]


class _NativeUnlockCandidate(ctypes.Structure):
    _fields_ = [
        ("parcel_id", ctypes.c_int),
        ("current_max_run", ctypes.c_double),
        ("simulated_max_run", ctypes.c_double),
        ("unlock_gain", ctypes.c_double),
    ]


@dataclass
class PCIReport:
    total_length: float
    total_ready_length: float
    total_blocked_length: float
    max_continuous_ready_length: float
    pci: float
    is_accelerated: bool

    def to_dict(self) -> Dict[str, Any]:
        return {
            "total_length": round(self.total_length, 4),
            "total_ready_length": round(self.total_ready_length, 4),
            "total_blocked_length": round(self.total_blocked_length, 4),
            "max_continuous_ready_length": round(self.max_continuous_ready_length, 4),
            "pci": round(self.pci, 4),
            "is_accelerated": self.is_accelerated,
        }


@dataclass
class UnlockRanking:
    parcel_id: int
    current_max_run: float
    simulated_max_run: float
    unlock_gain: float

    def to_dict(self) -> Dict[str, Any]:
        return {
            "parcel_id": self.parcel_id,
            "current_max_run": round(self.current_max_run, 4),
            "simulated_max_run": round(self.simulated_max_run, 4),
            "unlock_gain": round(self.unlock_gain, 4),
        }


class PCIService:
    """Service computing continuous corridor frontage and unlock bottlenecks."""

    def __init__(self, force_python: bool = False):
        self.force_python = force_python
        self._native_lib = None
        if not force_python:
            self._native_lib = self._load_or_build_native_lib()

    @property
    def is_native_available(self) -> bool:
        return self._native_lib is not None

    def _load_or_build_native_lib(self) -> Optional[ctypes.CDLL]:
        """Loads compiled C++ shared library or builds it if g++ is present."""
        base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../native"))
        core_name = "dharti_core.dll" if os.name == "nt" else "libdharti_core.so"
        core_path = os.path.join(base_dir, core_name)
        legacy_name = "pci_engine.dll" if os.name == "nt" else "libpci_engine.so"
        legacy_path = os.path.join(base_dir, legacy_name)
        dll_path = core_path if os.path.exists(core_path) else legacy_path
        cpp_path = os.path.join(base_dir, "pci_engine.cpp")

        # Attempt to compile if DLL missing but C++ source and g++ exist
        if not os.path.exists(dll_path) and os.path.exists(cpp_path):
            compiler = shutil.which("g++") or shutil.which("clang++")
            if compiler:
                try:
                    logger.info("Compiling native PCI engine via %s...", compiler)
                    cmd = [
                        compiler,
                        "-O3",
                        "-shared",
                        "-static",
                        "-static-libgcc",
                        "-static-libstdc++",
                        "-std=c++17",
                        "-o",
                        dll_path,
                        cpp_path,
                    ]
                    subprocess.run(cmd, check=True, capture_output=True)
                except Exception as ex:
                    logger.warning("Failed to compile native PCI engine: %s", ex)

        if os.path.exists(dll_path):
            try:
                lib = ctypes.CDLL(dll_path)
                # Signature: compute_pci_native
                lib.compute_pci_native.argtypes = [
                    ctypes.c_double,
                    ctypes.POINTER(_NativeChainageInterval),
                    ctypes.c_int,
                    ctypes.POINTER(_NativePCIResult),
                ]
                lib.compute_pci_native.restype = ctypes.c_int

                # Signature: simulate_unlock_native
                lib.simulate_unlock_native.argtypes = [
                    ctypes.c_double,
                    ctypes.POINTER(_NativeChainageInterval),
                    ctypes.c_int,
                    ctypes.POINTER(_NativeUnlockCandidate),
                    ctypes.c_int,
                    ctypes.POINTER(ctypes.c_int),
                ]
                lib.simulate_unlock_native.restype = ctypes.c_int

                logger.info("Native C++ PCI engine successfully loaded from %s", dll_path)
                return lib
            except Exception as e:
                logger.warning("Could not load native PCI library (%s): %s", dll_path, e)

        return None

    def compute_pci(
        self,
        total_length: float,
        intervals: List[Dict[str, Any]],
    ) -> PCIReport:
        """Computes the PCI metric and continuous frontage length."""
        if total_length <= 0:
            raise ValueError("total_length must be greater than 0")

        if self.is_native_available:
            return self._compute_pci_native(total_length, intervals)
        return self._compute_pci_python(total_length, intervals)

    def simulate_unlock(
        self,
        total_length: float,
        intervals: List[Dict[str, Any]],
        max_rankings: int = 10,
    ) -> List[UnlockRanking]:
        """Simulates unlocking each blocked parcel to find highest impact on continuous frontage."""
        if total_length <= 0:
            raise ValueError("total_length must be greater than 0")

        if self.is_native_available:
            return self._simulate_unlock_native(total_length, intervals, max_rankings)
        return self._simulate_unlock_python(total_length, intervals, max_rankings)

    # ---------------- Native Implementations ---------------- #

    def _compute_pci_native(
        self, total_length: float, intervals: List[Dict[str, Any]]
    ) -> PCIReport:
        n = len(intervals)
        c_intervals = (_NativeChainageInterval * n)()
        for i, item in enumerate(intervals):
            c_intervals[i].parcel_id = int(item.get("parcel_id", i))
            c_intervals[i].start_chainage = float(item.get("start_chainage", 0.0))
            c_intervals[i].end_chainage = float(item.get("end_chainage", 0.0))
            c_intervals[i].is_ready = 1 if item.get("is_ready", False) else 0

        result = _NativePCIResult()
        code = self._native_lib.compute_pci_native(
            ctypes.c_double(total_length),
            c_intervals,
            ctypes.c_int(n),
            ctypes.byref(result),
        )
        if code != 0:
            raise RuntimeError(f"Native compute_pci failed with code {code}")

        return PCIReport(
            total_length=result.total_length,
            total_ready_length=result.total_ready_length,
            total_blocked_length=result.total_blocked_length,
            max_continuous_ready_length=result.max_continuous_ready_length,
            pci=result.pci,
            is_accelerated=True,
        )

    def _simulate_unlock_native(
        self, total_length: float, intervals: List[Dict[str, Any]], max_rankings: int
    ) -> List[UnlockRanking]:
        n = len(intervals)
        c_intervals = (_NativeChainageInterval * n)()
        for i, item in enumerate(intervals):
            c_intervals[i].parcel_id = int(item.get("parcel_id", i))
            c_intervals[i].start_chainage = float(item.get("start_chainage", 0.0))
            c_intervals[i].end_chainage = float(item.get("end_chainage", 0.0))
            c_intervals[i].is_ready = 1 if item.get("is_ready", False) else 0

        capacity = min(n, max_rankings * 2) if n > 0 else 1
        c_candidates = (_NativeUnlockCandidate * capacity)()
        actual_count = ctypes.c_int(0)

        code = self._native_lib.simulate_unlock_native(
            ctypes.c_double(total_length),
            c_intervals,
            ctypes.c_int(n),
            c_candidates,
            ctypes.c_int(capacity),
            ctypes.byref(actual_count),
        )
        if code != 0:
            raise RuntimeError(f"Native simulate_unlock failed with code {code}")

        rankings = []
        for i in range(actual_count.value):
            cand = c_candidates[i]
            rankings.append(
                UnlockRanking(
                    parcel_id=cand.parcel_id,
                    current_max_run=cand.current_max_run,
                    simulated_max_run=cand.simulated_max_run,
                    unlock_gain=cand.unlock_gain,
                )
            )
        return rankings[:max_rankings]

    # ---------------- Pure Python Fallback Implementations ---------------- #

    @staticmethod
    def _merge_intervals(raw_intervals: List[Tuple[float, float]]) -> Tuple[float, float]:
        if not raw_intervals:
            return 0.0, 0.0

        sorted_intervals = sorted(raw_intervals, key=lambda x: (x[0], x[1]))
        total_ready = 0.0
        max_run = 0.0

        curr_start, curr_end = sorted_intervals[0]
        for start, end in sorted_intervals[1:]:
            if start <= curr_end + 1e-9:
                curr_end = max(curr_end, end)
            else:
                run_len = curr_end - curr_start
                total_ready += run_len
                if run_len > max_run:
                    max_run = run_len
                curr_start, curr_end = start, end

        last_run = curr_end - curr_start
        total_ready += last_run
        if last_run > max_run:
            max_run = last_run

        return total_ready, max_run

    def _compute_pci_python(
        self, total_length: float, intervals: List[Dict[str, Any]]
    ) -> PCIReport:
        ready_spans = [
            (float(item["start_chainage"]), float(item["end_chainage"]))
            for item in intervals
            if item.get("is_ready", False) and float(item["end_chainage"]) > float(item["start_chainage"])
        ]
        total_ready, max_run = self._merge_intervals(ready_spans)
        blocked = max(0.0, total_length - total_ready)
        pci = max_run / total_length if total_length > 0 else 0.0

        return PCIReport(
            total_length=total_length,
            total_ready_length=total_ready,
            total_blocked_length=blocked,
            max_continuous_ready_length=max_run,
            pci=pci,
            is_accelerated=False,
        )

    def _simulate_unlock_python(
        self, total_length: float, intervals: List[Dict[str, Any]], max_rankings: int
    ) -> List[UnlockRanking]:
        base_report = self._compute_pci_python(total_length, intervals)
        baseline_max = base_report.max_continuous_ready_length

        blocked_parcel_ids = {
            item["parcel_id"] for item in intervals if not item.get("is_ready", False)
        }

        candidates = []
        for pid in blocked_parcel_ids:
            sim_spans = [
                (float(item["start_chainage"]), float(item["end_chainage"]))
                for item in intervals
                if (item.get("is_ready", False) or item.get("parcel_id") == pid)
                and float(item["end_chainage"]) > float(item["start_chainage"])
            ]
            _, sim_max = self._merge_intervals(sim_spans)
            gain = sim_max - baseline_max
            candidates.append(
                UnlockRanking(
                    parcel_id=int(pid),
                    current_max_run=baseline_max,
                    simulated_max_run=sim_max,
                    unlock_gain=gain,
                )
            )

        candidates.sort(key=lambda c: (-c.unlock_gain, c.parcel_id))
        return candidates[:max_rankings]
