"""Baseline tests verifying modular architecture setup."""

import unittest
from config.settings import config
from src.utils.logger import get_logger
from src.core.base import BaseService

class TestBaselineSetup(unittest.TestCase):
    """Verify core imports, configuration, and logging functionality."""

    def test_config_initialization(self):
        self.assertEqual(config.app_name, "DHARTI")
        self.assertIsNotNone(config.app_env)

    def test_logger_initialization(self):
        logger = get_logger("test_logger")
        self.assertEqual(logger.name, "test_logger")

    def test_base_service_interface(self):
        class DummyService(BaseService):
            def health_check(self):
                return {"status": "ok"}

        service = DummyService()
        self.assertEqual(service.health_check(), {"status": "ok"})

if __name__ == "__main__":
    unittest.main()
