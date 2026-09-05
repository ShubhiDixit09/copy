"""Unit tests for PCI calculation and unlock simulation (C++ Native and Python Fallback)."""

import unittest
from src.services.pci.pci_service import PCIService

class TestPCIEngine(unittest.TestCase):
    """Test suite comparing Python and C++ accelerated PCI computations."""

    def setUp(self):
        # Synthetic corridor: 12 km alignment with 5 parcel intervals
        # Parcels:
        # P-101: 0.0 -> 3.0 km (Ready)
        # P-102: 3.0 -> 5.0 km (Ready)  -> continuous run 5.0 km
        # P-118: 5.0 -> 6.5 km (Blocked by litigation/succession)
        # P-104: 6.5 -> 10.0 km (Ready) -> continuous run 3.5 km
        # P-105: 10.0 -> 12.0 km (Blocked)
        self.total_length = 12.0
        self.intervals = [
            {"parcel_id": 101, "start_chainage": 0.0, "end_chainage": 3.0, "is_ready": True},
            {"parcel_id": 102, "start_chainage": 3.0, "end_chainage": 5.0, "is_ready": True},
            {"parcel_id": 118, "start_chainage": 5.0, "end_chainage": 6.5, "is_ready": False},
            {"parcel_id": 104, "start_chainage": 6.5, "end_chainage": 10.0, "is_ready": True},
            {"parcel_id": 105, "start_chainage": 10.0, "end_chainage": 12.0, "is_ready": False},
        ]

    def test_python_pci_calculation(self):
        service = PCIService(force_python=True)
        report = service.compute_pci(self.total_length, self.intervals)

        self.assertFalse(report.is_accelerated)
        self.assertAlmostEqual(report.total_ready_length, 8.5) # 5.0 + 3.5
        self.assertAlmostEqual(report.total_blocked_length, 3.5) # 1.5 + 2.0
        self.assertAlmostEqual(report.max_continuous_ready_length, 5.0) # 0.0 to 5.0
        self.assertAlmostEqual(report.pci, 5.0 / 12.0)

    def test_python_unlock_simulation(self):
        service = PCIService(force_python=True)
        rankings = service.simulate_unlock(self.total_length, self.intervals)

        self.assertEqual(len(rankings), 2)
        top_parcel = rankings[0]
        # P-118 bridges [0.0, 5.0] and [6.5, 10.0] -> creates continuous [0.0, 10.0] = 10.0 km
        # Unlock gain = 10.0 - 5.0 = 5.0 km!
        self.assertEqual(top_parcel.parcel_id, 118)
        self.assertAlmostEqual(top_parcel.simulated_max_run, 10.0)
        self.assertAlmostEqual(top_parcel.unlock_gain, 5.0)

        second_parcel = rankings[1]
        self.assertEqual(second_parcel.parcel_id, 105)
        # P-105 attaches [10.0, 12.0] to [6.5, 10.0] -> creates continuous [6.5, 12.0] = 5.5 km
        # Unlock gain = 5.5 - 5.0 = 0.5 km
        self.assertAlmostEqual(second_parcel.unlock_gain, 0.5)

    def test_native_cpp_vs_python_parity(self):
        py_service = PCIService(force_python=True)
        native_service = PCIService(force_python=False)

        if not native_service.is_native_available:
            self.skipTest("C++ compiler or native DLL not available in environment")

        py_report = py_service.compute_pci(self.total_length, self.intervals)
        cpp_report = native_service.compute_pci(self.total_length, self.intervals)

        self.assertTrue(cpp_report.is_accelerated)
        self.assertAlmostEqual(py_report.total_ready_length, cpp_report.total_ready_length)
        self.assertAlmostEqual(py_report.max_continuous_ready_length, cpp_report.max_continuous_ready_length)
        self.assertAlmostEqual(py_report.pci, cpp_report.pci)

        py_rankings = py_service.simulate_unlock(self.total_length, self.intervals)
        cpp_rankings = native_service.simulate_unlock(self.total_length, self.intervals)

        self.assertEqual(len(py_rankings), len(cpp_rankings))
        for py_rank, cpp_rank in zip(py_rankings, cpp_rankings):
            self.assertEqual(py_rank.parcel_id, cpp_rank.parcel_id)
            self.assertAlmostEqual(py_rank.unlock_gain, cpp_rank.unlock_gain)

if __name__ == "__main__":
    unittest.main()
