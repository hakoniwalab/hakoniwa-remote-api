from __future__ import annotations

import json
import os
from dataclasses import dataclass
from typing import Any, Dict, List


@dataclass
class LintError:
    """Represents a single linting error."""
    msg: str

    def __str__(self) -> str:
        return self.msg


class LinterBase:
    """Base class for linters."""
    def __init__(self, file_path: str):
        self.file_path = file_path
        self.errors: List[LintError] = []
        try:
            self.data = self._load_json(file_path)
        except (IOError, json.JSONDecodeError) as e:
            self.errors.append(LintError(f"Failed to read or parse {file_path}: {e}"))
            self.data = None

    def _load_json(self, path: str) -> Dict[str, Any]:
        with open(path, "r", encoding="utf-8") as f:
            return json.load(f)

    def _resolve_path(self, maybe_rel: str) -> str:
        """Resolve path relative to the directory containing the linter's base file."""
        if os.path.isabs(maybe_rel):
            return os.path.normpath(maybe_rel)
        return os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(self.file_path)), maybe_rel))

    def _require_type(self, obj: Any, t: type, ctx: str) -> bool:
        if not isinstance(obj, t):
            self.errors.append(LintError(f"{ctx}: expected {t.__name__}, got {type(obj).__name__}"))
            return False
        return True

    def lint(self) -> List[LintError]:
        """Runs all linting checks for the file."""
        if self.data is not None:
            self._run_checks()
        return self.errors

    def _run_checks(self):
        """Subclasses must implement their specific checks here."""
        raise NotImplementedError
