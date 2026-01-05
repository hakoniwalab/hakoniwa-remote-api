from __future__ import annotations

import os
from typing import List

from .base import LinterBase, LintError


class EndpointLinter(LinterBase):
    """Linter for endpoint JSON files (e.g., server_endpoint.json)."""

    def _run_checks(self):
        """Checks for file existence of paths within the endpoint file."""
        paths_to_check = ["pdu_def_path", "cache", "comm"]
        
        for key in paths_to_check:
            relative_path = self.data.get(key)
            if self._require_type(relative_path, str, f"endpoint.{key}") and relative_path:
                resolved_path = self._resolve_path(relative_path)
                if not os.path.exists(resolved_path):
                    self.errors.append(LintError(
                        f"{os.path.basename(self.file_path)}: path for '{key}' not found: '{relative_path}' (resolved: '{resolved_path}')"
                    ))
            elif not relative_path:
                self.errors.append(LintError(f"endpoint.{key}: missing or empty string"))
