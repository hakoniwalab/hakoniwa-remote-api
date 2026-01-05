#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Hakoniwa config linker/linter (cross-file consistency checker)

- No external dependencies (stdlib only)
- Intended to run AFTER ajv schema validation

Checks:
- remote-api.json:
  - participants[].name uniqueness
  - rpc_service_config_path exists (resolved relative to remote-api.json)
- rpc.json:
  - endpoints reference resolution:
      services[].server_endpoint.nodeId exists in rpc.endpoints[].nodeId
      server_endpoint.endpointId exists under that node
      clients[].client_endpoint.nodeId exists
      clients[].client_endpoint.endpointId exists under that node
  - clients.length <= maxClients
  - channelId collisions inside a service
  - endpoints[].endpoints[].config_path existence (resolved relative to rpc.json)
  - pdu_config_path existence (resolved relative to rpc.json)
"""

from __future__ import annotations

import argparse
import json
import os
import sys
from dataclasses import dataclass
from typing import Any, Dict, List, Set


@dataclass
class LintError:
    msg: str

    def __str__(self) -> str:
        return self.msg


def load_json(path: str) -> Dict[str, Any]:
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def resolve_path(base_file: str, maybe_rel: str) -> str:
    """Resolve path relative to the directory containing base_file."""
    if os.path.isabs(maybe_rel):
        return os.path.normpath(maybe_rel)
    return os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(base_file)), maybe_rel))


def require_type(errors: List[LintError], obj: Any, t, ctx: str) -> bool:
    if not isinstance(obj, t):
        errors.append(LintError(f"{ctx}: expected {t.__name__}, got {type(obj).__name__}"))
        return False
    return True


def build_endpoint_index(rpc: Dict[str, Any], errors: List[LintError]) -> Dict[str, Set[str]]:
    """
    Build nodeId -> set(endpointId) index from rpc['endpoints'].
    Also checks duplicate endpoint IDs within a node (warn as error).
    """
    idx: Dict[str, Set[str]] = {}

    eps = rpc.get("endpoints")
    if not require_type(errors, eps, list, "rpc.endpoints"):
        return idx

    for i, node in enumerate(eps):
        if not isinstance(node, dict):
            errors.append(LintError(f"rpc.endpoints[{i}]: expected object"))
            continue
        node_id = node.get("nodeId")
        if not isinstance(node_id, str) or not node_id:
            errors.append(LintError(f"rpc.endpoints[{i}].nodeId: missing or not string"))
            continue

        idx.setdefault(node_id, set())
        node_eps = node.get("endpoints")
        if not require_type(errors, node_eps, list, f"rpc.endpoints[{i}].endpoints"):
            continue

        for j, ep in enumerate(node_eps):
            if not isinstance(ep, dict):
                errors.append(LintError(f"rpc.endpoints[{i}].endpoints[{j}]: expected object"))
                continue
            ep_id = ep.get("id")
            if not isinstance(ep_id, str) or not ep_id:
                errors.append(LintError(f"rpc.endpoints[{i}].endpoints[{j}].id: missing or not string"))
                continue
            if ep_id in idx[node_id]:
                errors.append(LintError(f"rpc.endpoints[{i}] node '{node_id}': duplicate endpoint id '{ep_id}'"))
            idx[node_id].add(ep_id)

    return idx


def lint_remote_api(remote_path: str, remote: Dict[str, Any]) -> List[LintError]:
    errors: List[LintError] = []

    # rpc_service_config_path existence
    rpc_path = remote.get("rpc_service_config_path")
    if not isinstance(rpc_path, str) or not rpc_path:
        errors.append(LintError("remote-api.rpc_service_config_path: missing or not string"))
    else:
        resolved = resolve_path(remote_path, rpc_path)
        if not os.path.exists(resolved):
            errors.append(LintError(
                f"remote-api.rpc_service_config_path: not found: '{rpc_path}' (resolved: '{resolved}')"
            ))

    # participants.name uniqueness
    parts = remote.get("participants")
    if require_type(errors, parts, list, "remote-api.participants"):
        seen: Set[str] = set()
        for i, p in enumerate(parts):
            if not isinstance(p, dict):
                errors.append(LintError(f"remote-api.participants[{i}]: expected object"))
                continue
            name = p.get("name")
            if not isinstance(name, str) or not name:
                errors.append(LintError(f"remote-api.participants[{i}].name: missing or not string"))
                continue
            if name in seen:
                errors.append(LintError(f"remote-api.participants: duplicate name '{name}'"))
            seen.add(name)

    return errors


def lint_rpc(rpc_path: str, rpc: Dict[str, Any]) -> List[LintError]:
    errors: List[LintError] = []

    # pdu_config_path existence
    pdu_path = rpc.get("pdu_config_path")
    if isinstance(pdu_path, str) and pdu_path:
        resolved = resolve_path(rpc_path, pdu_path)
        if not os.path.exists(resolved):
            errors.append(LintError(
                f"rpc.pdu_config_path: not found: '{pdu_path}' (resolved: '{resolved}')"
            ))
    else:
        errors.append(LintError("rpc.pdu_config_path: missing or not string"))

    endpoint_idx = build_endpoint_index(rpc, errors)

    # endpoints[*].endpoints[*].config_path existence
    eps = rpc.get("endpoints")
    if isinstance(eps, list):
        for i, node in enumerate(eps):
            if not isinstance(node, dict):
                continue
            node_id = node.get("nodeId")
            node_eps = node.get("endpoints")
            if not isinstance(node_eps, list):
                continue
            for j, ep in enumerate(node_eps):
                if not isinstance(ep, dict):
                    continue
                cp = ep.get("config_path")
                if not isinstance(cp, str) or not cp:
                    errors.append(LintError(
                        f"rpc.endpoints[{i}].endpoints[{j}].config_path: missing or not string"
                    ))
                    continue
                resolved = resolve_path(rpc_path, cp)
                if not os.path.exists(resolved):
                    errors.append(LintError(
                        f"rpc.endpoints[{i}] node '{node_id}': config_path not found: '{cp}' (resolved: '{resolved}')"
                    ))

    # services references + channel collision
    svcs = rpc.get("services")
    if not require_type(errors, svcs, list, "rpc.services"):
        return errors

    service_names: Set[str] = set()
    for si, svc in enumerate(svcs):
        if not isinstance(svc, dict):
            errors.append(LintError(f"rpc.services[{si}]: expected object"))
            continue

        sname = svc.get("name")
        if isinstance(sname, str) and sname:
            if sname in service_names:
                errors.append(LintError(f"rpc.services: duplicate service name '{sname}'"))
            service_names.add(sname)
        else:
            sname = f"<services[{si}]>"

        # clients.length <= maxClients
        max_clients = svc.get("maxClients")
        clients = svc.get("clients")
        if isinstance(max_clients, int) and isinstance(clients, list):
            if len(clients) > max_clients:
                errors.append(LintError(
                    f"rpc.services[{si}] '{sname}': clients.length({len(clients)}) > maxClients({max_clients})"
                ))

        # server_endpoint reference
        se = svc.get("server_endpoint")
        if not isinstance(se, dict):
            errors.append(LintError(f"rpc.services[{si}] '{sname}': server_endpoint missing or not object"))
        else:
            snode = se.get("nodeId")
            seid = se.get("endpointId")
            if not (isinstance(snode, str) and snode):
                errors.append(LintError(f"rpc.services[{si}] '{sname}': server_endpoint.nodeId missing or not string"))
            elif snode not in endpoint_idx:
                errors.append(LintError(f"rpc.services[{si}] '{sname}': server nodeId '{snode}' not found in rpc.endpoints"))
            if not (isinstance(seid, str) and seid):
                errors.append(LintError(f"rpc.services[{si}] '{sname}': server_endpoint.endpointId missing or not string"))
            elif isinstance(snode, str) and snode in endpoint_idx and seid not in endpoint_idx[snode]:
                errors.append(LintError(
                    f"rpc.services[{si}] '{sname}': server endpointId '{seid}' not found under node '{snode}'"
                ))

        # client endpoints + channel collision
        used_channels: Set[int] = set()
        client_names: Set[str] = set()

        if isinstance(clients, list):
            for ci, c in enumerate(clients):
                if not isinstance(c, dict):
                    errors.append(LintError(f"rpc.services[{si}] '{sname}': clients[{ci}] expected object"))
                    continue

                cname = c.get("name")
                if isinstance(cname, str) and cname:
                    if cname in client_names:
                        errors.append(LintError(
                            f"rpc.services[{si}] '{sname}': duplicate client name '{cname}'"
                        ))
                    client_names.add(cname)

                req = c.get("requestChannelId")
                res = c.get("responseChannelId")
                if isinstance(req, int):
                    if req in used_channels:
                        errors.append(LintError(
                            f"rpc.services[{si}] '{sname}': channel collision: {req} (requestChannelId)"
                        ))
                    used_channels.add(req)
                else:
                    errors.append(LintError(
                        f"rpc.services[{si}] '{sname}': clients[{ci}].requestChannelId missing or not int"
                    ))
                if isinstance(res, int):
                    if res in used_channels:
                        errors.append(LintError(
                            f"rpc.services[{si}] '{sname}': channel collision: {res} (responseChannelId)"
                        ))
                    used_channels.add(res)
                else:
                    errors.append(LintError(
                        f"rpc.services[{si}] '{sname}': clients[{ci}].responseChannelId missing or not int"
                    ))

                ce = c.get("client_endpoint")
                if not isinstance(ce, dict):
                    errors.append(LintError(
                        f"rpc.services[{si}] '{sname}': clients[{ci}].client_endpoint missing or not object"
                    ))
                    continue

                cnode = ce.get("nodeId")
                ceid = ce.get("endpointId")

                if not (isinstance(cnode, str) and cnode):
                    errors.append(LintError(
                        f"rpc.services[{si}] '{sname}': clients[{ci}].client_endpoint.nodeId missing or not string"
                    ))
                elif cnode not in endpoint_idx:
                    errors.append(LintError(
                        f"rpc.services[{si}] '{sname}': client nodeId '{cnode}' not found in rpc.endpoints"
                    ))

                if not (isinstance(ceid, str) and ceid):
                    errors.append(LintError(
                        f"rpc.services[{si}] '{sname}': clients[{ci}].client_endpoint.endpointId missing or not string"
                    ))
                elif isinstance(cnode, str) and cnode in endpoint_idx and ceid not in endpoint_idx[cnode]:
                    errors.append(LintError(
                        f"rpc.services[{si}] '{sname}': client endpointId '{ceid}' not found under node '{cnode}'"
                    ))

    return errors


def main() -> int:
    ap = argparse.ArgumentParser(description="Hakoniwa config cross-file linter (no deps)")
    ap.add_argument("remote_api_json", help="path to remote-api.json")
    args = ap.parse_args()

    remote_path = args.remote_api_json
    remote = load_json(remote_path)

    errors: List[LintError] = []
    errors.extend(lint_remote_api(remote_path, remote))

    rpc_rel = remote.get("rpc_service_config_path")
    if isinstance(rpc_rel, str) and rpc_rel:
        rpc_path = resolve_path(remote_path, rpc_rel)
        if os.path.exists(rpc_path):
            rpc = load_json(rpc_path)
            errors.extend(lint_rpc(rpc_path, rpc))

    if errors:
        print("LINT FAILED:")
        for e in errors:
            print(" - " + str(e))
        return 1

    print("LINT OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
