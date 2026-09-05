"""Configuration management for DHARTI."""

import os
from dataclasses import dataclass

@dataclass(frozen=True)
class AppConfig:
    """Application configuration container."""
    app_name: str = os.getenv("DHARTI_APP_NAME", "DHARTI")
    app_env: str = os.getenv("DHARTI_ENV", "development")
    debug: bool = os.getenv("DHARTI_DEBUG", "True").lower() in ("true", "1", "yes")
    log_level: str = os.getenv("DHARTI_LOG_LEVEL", "INFO")

config = AppConfig()
