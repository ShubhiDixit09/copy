"""Base abstractions and interfaces for DHARTI."""

from abc import ABC, abstractmethod
from typing import Any, Dict

class BaseService(ABC):
    """Abstract base class for all domain services."""
    
    @abstractmethod
    def health_check(self) -> Dict[str, Any]:
        """Verify service operational status."""
        pass
