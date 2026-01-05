from __future__ import annotations

import os
from typing import Dict, List, Optional, Set

from .base import LinterBase, LintError


class RemoteApiLinter(LinterBase):
    """Linter for remote-api.json."""

    def _run_checks(self):
        self._check_rpc_service_config_path()
        self._check_participants()

    def _check_rpc_service_config_path(self):
        rpc_path = self.data.get("rpc_service_config_path")
        if not self._require_type(rpc_path, str, "remote-api.rpc_service_config_path"):
            return

        resolved = self._resolve_path(rpc_path)
        if not os.path.exists(resolved):
            self.errors.append(LintError(
                f"remote-api.rpc_service_config_path: not found: '{rpc_path}' (resolved: '{resolved}')"
            ))

    def _check_participants(self):
        parts = self.data.get("participants")
        if not self._require_type(parts, list, "remote-api.participants"):
            return

        seen_names: Set[str] = set()
        seen_node_ids: Set[str] = set()
        for i, p in enumerate(parts):
            ctx = f"remote-api.participants[{i}]"
            if not self._require_type(p, dict, ctx):
                continue

            # Check name
            name = p.get("name")
            if not self._require_type(name, str, f"{ctx}.name") or not name:
                self.errors.append(LintError(f"{ctx}.name: missing or empty string"))
            elif name in seen_names:
                self.errors.append(LintError(f"remote-api.participants: duplicate name '{name}'"))
            seen_names.add(name)

            # Check nodeId
            node_id = p.get("nodeId")
            if not self._require_type(node_id, str, f"{ctx}.nodeId") or not node_id:
                self.errors.append(LintError(f"{ctx}.nodeId: missing or empty string"))
            elif node_id in seen_node_ids:
                self.errors.append(LintError(f"remote-api.participants: duplicate nodeId '{node_id}'"))
            seen_node_ids.add(node_id)


    def get_rpc_path(self) -> Optional[str]:
        if self.data:
            return self.data.get("rpc_service_config_path")
        return None

    def get_participants(self) -> List[Dict]:
        if self.data:
            return self.data.get("participants", [])
        return []
