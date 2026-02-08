#!/usr/bin/env python3
"""
Hakoniwa Remote API config validator.

- Validates remote-api.json structure
- Ensures referenced rpc.json exists
- Cross-checks participant nodeIds against rpc.json endpoints
"""

from __future__ import annotations

import argparse
import json
import os
from dataclasses import dataclass
from typing import Any, Dict, List, Optional, Set


@dataclass
class LintError:
    msg: str

    def __str__(self) -> str:
        return self.msg


class LinterBase:
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
        if os.path.isabs(maybe_rel):
            return os.path.normpath(maybe_rel)
        return os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(self.file_path)), maybe_rel))

    def _require_type(self, obj: Any, t: type, ctx: str) -> bool:
        if not isinstance(obj, t):
            self.errors.append(LintError(f"{ctx}: expected {t.__name__}, got {type(obj).__name__}"))
            return False
        return True

    def lint(self) -> List[LintError]:
        if self.data is not None:
            self._run_checks()
        return self.errors

    def _run_checks(self):
        raise NotImplementedError


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

            name = p.get("name")
            if not self._require_type(name, str, f"{ctx}.name") or not name:
                self.errors.append(LintError(f"{ctx}.name: missing or empty string"))
            elif name in seen_names:
                self.errors.append(LintError(f"remote-api.participants: duplicate name '{name}'"))
            seen_names.add(name)

            node_id = p.get("nodeId")
            if not self._require_type(node_id, str, f"{ctx}.nodeId") or not node_id:
                self.errors.append(LintError(f"{ctx}.nodeId: missing or empty string"))
            elif node_id in seen_node_ids:
                self.errors.append(LintError(f"remote-api.participants: duplicate nodeId '{node_id}'"))
            seen_node_ids.add(node_id)

            if server_node_ids is not None:
                server_node_id = p.get("server_nodeId")
                if not self._require_type(server_node_id, str, f"{ctx}.server_nodeId") or not server_node_id:
                    self.errors.append(LintError(f"{ctx}.server_nodeId: missing or empty string"))
                elif server_node_id not in server_node_ids:
                    self.errors.append(LintError(
                        f"{ctx}.server_nodeId '{server_node_id}': not found in remote-api.servers"
                    ))

            role = p.get("role")
            if not self._require_type(role, str, f"{ctx}.role") or not role:
                self.errors.append(LintError(f"{ctx}.role: missing or empty string"))
            elif role not in {"conductor", "asset"}:
                self.errors.append(LintError(f"{ctx}.role: invalid value '{role}'"))

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


def _collect_rpc_node_ids(rpc_json) -> Set[str]:
    node_ids: Set[str] = set()
    endpoints = rpc_json.get("endpoints")
    if isinstance(endpoints, list):
        for node in endpoints:
            if isinstance(node, dict):
                node_id = node.get("nodeId")
                if isinstance(node_id, str) and node_id:
                    node_ids.add(node_id)
    return node_ids


def _check_cross_file_references(remote_linter: RemoteApiLinter, rpc_node_ids: Set[str]) -> List[LintError]:
    errors: List[LintError] = []

    participants = remote_linter.get_participants()

    for i, p in enumerate(participants):
        node_id = p.get("nodeId")
        if isinstance(node_id, str) and node_id:
            if node_id not in rpc_node_ids:
                errors.append(LintError(
                    f"remote-api.participants[{i}]: nodeId '{node_id}' not found in rpc.json's endpoints"
                ))

    return errors


def main() -> int:
    ap = argparse.ArgumentParser(description="Hakoniwa remote-api config linter")
    ap.add_argument("remote_api_json", help="path to remote-api.json")
    args = ap.parse_args()

    all_errors: List[LintError] = []

    remote_linter = RemoteApiLinter(args.remote_api_json)
    all_errors.extend(remote_linter.lint())

    rpc_node_ids: Set[str] = set()
    if remote_linter.data:
        rpc_rel = remote_linter.get_rpc_path()
        if isinstance(rpc_rel, str) and rpc_rel:
            base_dir = os.path.dirname(os.path.abspath(args.remote_api_json))
            rpc_path = os.path.normpath(os.path.join(base_dir, rpc_rel))
            if not os.path.exists(rpc_path):
                all_errors.append(LintError(
                    f"remote-api.rpc_service_config_path: not found: '{rpc_rel}' (resolved: '{rpc_path}')"
                ))
            else:
                try:
                    with open(rpc_path, "r", encoding="utf-8") as f:
                        rpc_json = json.load(f)
                    rpc_node_ids = _collect_rpc_node_ids(rpc_json)
                except (OSError, json.JSONDecodeError) as e:
                    all_errors.append(LintError(
                        f"remote-api.rpc_service_config_path: failed to read {rpc_path}: {e}"
                    ))

    if remote_linter.data and rpc_node_ids:
        all_errors.extend(
            _check_cross_file_references(remote_linter, rpc_node_ids)
        )

    if all_errors:
        print("LINT FAILED:")
        for e in all_errors:
            print(f" - {e}")
        return 1

    print("LINT OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
