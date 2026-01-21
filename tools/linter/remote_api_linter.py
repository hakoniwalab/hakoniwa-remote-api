from __future__ import annotations

import os
from typing import Dict, List, Optional, Set

from .base import LinterBase, LintError


class RemoteApiLinter(LinterBase):
    """Linter for remote-api.json."""

    def _run_checks(self):
        self._check_rpc_service_config_path()
        self._check_servers()
        self._check_time_settings()
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

        server_node_ids = self._get_server_node_ids()
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

            # Check server_nodeId
            if server_node_ids is not None:
                server_node_id = p.get("server_nodeId")
                if not self._require_type(server_node_id, str, f"{ctx}.server_nodeId") or not server_node_id:
                    self.errors.append(LintError(f"{ctx}.server_nodeId: missing or empty string"))
                elif server_node_id not in server_node_ids:
                    self.errors.append(LintError(
                        f"{ctx}.server_nodeId '{server_node_id}': not found in remote-api.servers"
                    ))

            # Check role
            role = p.get("role")
            if not self._require_type(role, str, f"{ctx}.role") or not role:
                self.errors.append(LintError(f"{ctx}.role: missing or empty string"))
            elif role not in {"conductor", "asset"}:
                self.errors.append(LintError(f"{ctx}.role: invalid value '{role}'"))

            # Check poll_sleep_time_usec
            delta_time = p.get("poll_sleep_time_usec")
            if not self._require_type(delta_time, int, f"{ctx}.poll_sleep_time_usec"):
                continue
            if delta_time <= 0:
                self.errors.append(LintError(f"{ctx}.poll_sleep_time_usec: must be > 0"))

    def _check_servers(self):
        servers = self.data.get("servers")
        if not self._require_type(servers, list, "remote-api.servers"):
            return

        seen_node_ids: Set[str] = set()
        for i, server in enumerate(servers):
            ctx = f"remote-api.servers[{i}]"
            if not self._require_type(server, dict, ctx):
                continue

            node_id = server.get("nodeId")
            if not self._require_type(node_id, str, f"{ctx}.nodeId") or not node_id:
                self.errors.append(LintError(f"{ctx}.nodeId: missing or empty string"))
            elif node_id in seen_node_ids:
                self.errors.append(LintError(f"remote-api.servers: duplicate nodeId '{node_id}'"))
            seen_node_ids.add(node_id)

    def _check_time_settings(self):
        time_source = self.data.get("time_source_type")
        if not self._require_type(time_source, str, "remote-api.time_source_type"):
            return
        if time_source not in {"real", "virtual", "hakoniwa"}:
            self.errors.append(LintError(f"remote-api.time_source_type: invalid value '{time_source}'"))

        poll_sleep_time = self.data.get("poll_sleep_time_usec")
        if self._require_type(poll_sleep_time, int, "remote-api.poll_sleep_time_usec") and poll_sleep_time <= 0:
            self.errors.append(LintError("remote-api.poll_sleep_time_usec: must be > 0"))

    def _get_server_node_ids(self) -> Optional[Set[str]]:
        servers = self.data.get("servers")
        if not isinstance(servers, list):
            return None

        node_ids: Set[str] = set()
        for server in servers:
            if isinstance(server, dict):
                node_id = server.get("nodeId")
                if isinstance(node_id, str) and node_id:
                    node_ids.add(node_id)
        return node_ids


    def get_rpc_path(self) -> Optional[str]:
        if self.data:
            return self.data.get("rpc_service_config_path")
        return None

    def get_participants(self) -> List[Dict]:
        if self.data:
            return self.data.get("participants", [])
        return []
