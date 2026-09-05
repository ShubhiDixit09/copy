"""Logging utility for DHARTI."""

import logging
import sys
from config.settings import config

def get_logger(name: str) -> logging.Logger:
    """Returns a configured logger instance."""
    logger = logging.getLogger(name)
    if not logger.handlers:
        handler = logging.StreamHandler(sys.stdout)
        formatter = logging.Formatter(
            "[%(asctime)s] [%(levelname)s] [%(name)s]: %(message)s",
            datefmt="%Y-%m-%d %H:%M:%S",
        )
        handler.setFormatter(formatter)
        logger.addHandler(handler)
        logger.setLevel(getattr(logging, config.log_level.upper(), logging.INFO))
    return logger
