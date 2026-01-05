from __future__ import annotations

import os
from typing import Dict, List, Set

from .base import LinterBase, LintError
from .endpoint_linter import EndpointLinter


class RpcLinter(LinterBase):
    """Linter for rpc.json."""

    def __init__(self, file_path: str):
        super().__init__(file_path)
        self.endpoint_idx: Dict[str, Set[str]] = {}

    def _run_checks(self):
        self.endpoint_idx = self._build_endpoint_index()
        self._check_pdu_config_path()
        self._check_endpoint_config_paths()
        self._check_services()

    def _check_pdu_config_path(self):
        pdu_path = self.data.get("pdu_config_path")
        if self._require_type(pdu_path, str, "rpc.pdu_config_path") and pdu_path:
            resolved = self._resolve_path(pdu_path)
            if not os.path.exists(resolved):
                self.errors.append(LintError(
                    f"rpc.pdu_config_path: not found: '{pdu_path}' (resolved: '{resolved}')"
                ))
        elif not pdu_path:
             self.errors.append(LintError("rpc.pdu_config_path: missing or empty string"))


    def _build_endpoint_index(self) -> Dict[str, Set[str]]:
        idx: Dict[str, Set[str]] = {}
        eps = self.data.get("endpoints")
        if not self._require_type(eps, list, "rpc.endpoints"):
            return idx

        for i, node in enumerate(eps):
            if not self._require_type(node, dict, f"rpc.endpoints[{i}]"):
                continue
            node_id = node.get("nodeId")
            if not self._require_type(node_id, str, f"rpc.endpoints[{i}].nodeId") or not node_id:
                self.errors.append(LintError(f"rpc.endpoints[{i}].nodeId: missing or empty string"))
                continue

            idx.setdefault(node_id, set())
            node_eps = node.get("endpoints")
            if not self._require_type(node_eps, list, f"rpc.endpoints[{i}].endpoints"):
                continue

            for j, ep in enumerate(node_eps):
                if not self._require_type(ep, dict, f"rpc.endpoints[{i}].endpoints[{j}]"):
                    continue
                ep_id = ep.get("id")
                if not self._require_type(ep_id, str, f"rpc.endpoints[{i}].endpoints[{j}].id") or not ep_id:
                    self.errors.append(LintError(f"rpc.endpoints[{i}].endpoints[{j}].id: missing or empty string"))
                    continue
                if ep_id in idx[node_id]:
                    self.errors.append(LintError(f"rpc.endpoints[{i}] node '{node_id}': duplicate endpoint id '{ep_id}'"))
                idx[node_id].add(ep_id)
        return idx

    def _check_endpoint_config_paths(self):
        eps = self.data.get("endpoints")
        if not isinstance(eps, list): return

        for i, node in enumerate(eps):
            if not isinstance(node, dict): continue
            node_id = node.get("nodeId", f"<endpoints[{i}]>")
            node_eps = node.get("endpoints")
            if not isinstance(node_eps, list): continue

            for j, ep in enumerate(node_eps):
                if not isinstance(ep, dict): continue
                cp = ep.get("config_path")
                if not self._require_type(cp, str, f"rpc.endpoints[{i}].endpoints[{j}].config_path") or not cp:
                    self.errors.append(LintError(f"rpc.endpoints[{i}].endpoints[{j}].config_path: missing or empty string"))
                    continue
                
                resolved_path = self._resolve_path(cp)
                if not os.path.exists(resolved_path):
                    self.errors.append(LintError(
                        f"rpc.endpoints[{i}] node '{node_id}': config_path not found: '{cp}' (resolved: '{resolved_path}')"
                    ))
                else:
                    # Lint the endpoint file itself
                    ep_linter = EndpointLinter(resolved_path)
                    self.errors.extend(ep_linter.lint())


    def _check_services(self):
        svcs = self.data.get("services")
        if not self._require_type(svcs, list, "rpc.services"):
            return

        service_names: Set[str] = set()
        for si, svc in enumerate(svcs):
            if not self._require_type(svc, dict, f"rpc.services[{si}]"):
                continue

            sname = svc.get("name")
            if isinstance(sname, str) and sname:
                if sname in service_names:
                    self.errors.append(LintError(f"rpc.services: duplicate service name '{sname}'"))
                service_names.add(sname)
            else:
                sname = f"<services[{si}]>"

            self._check_service_clients_length(svc, si, sname)
            self._check_service_server_endpoint(svc, si, sname)
            self._check_service_clients(svc, si, sname)

    def _check_service_clients_length(self, svc: Dict, si: int, sname: str):
        max_clients = svc.get("maxClients")
        clients = svc.get("clients")
        if isinstance(max_clients, int) and isinstance(clients, list):
            if len(clients) > max_clients:
                self.errors.append(LintError(
                    f"rpc.services[{si}] '{sname}': clients.length({len(clients)}) > maxClients({max_clients})"
                ))

    def _check_service_server_endpoint(self, svc: Dict, si: int, sname: str):
        se = svc.get("server_endpoint")
        if not self._require_type(se, dict, f"rpc.services[{si}] '{sname}': server_endpoint"):
            return

        snode = se.get("nodeId")
        seid = se.get("endpointId")
        if not (self._require_type(snode, str, f"rpc.services[{si}] '{sname}': server_endpoint.nodeId") and snode): return
        if not (self._require_type(seid, str, f"rpc.services[{si}] '{sname}': server_endpoint.endpointId") and seid): return

        if snode not in self.endpoint_idx:
            self.errors.append(LintError(f"rpc.services[{si}] '{sname}': server nodeId '{snode}' not found in rpc.endpoints"))
        elif seid not in self.endpoint_idx[snode]:
            self.errors.append(LintError(f"rpc.services[{si}] '{sname}': server endpointId '{seid}' not found under node '{snode}'"))

    def _check_service_clients(self, svc: Dict, si: int, sname: str):
        clients = svc.get("clients")
        if not isinstance(clients, list): return

        used_channels: Set[int] = set()
        client_names: Set[str] = set()

        for ci, c in enumerate(clients):
            if not self._require_type(c, dict, f"rpc.services[{si}] '{sname}': clients[{ci}]"):
                continue

            cname = c.get("name")
            if isinstance(cname, str) and cname:
                if cname in client_names:
                    self.errors.append(LintError(f"rpc.services[{si}] '{sname}': duplicate client name '{cname}'"))
                client_names.add(cname)

            self._check_client_channels(c, si, ci, sname, used_channels)
            self._check_client_endpoint(c, si, ci, sname)

    def _check_client_channels(self, c: Dict, si: int, ci: int, sname: str, used_channels: Set[int]):
        for key in ["requestChannelId", "responseChannelId"]:
            channel_id = c.get(key)
            if self._require_type(channel_id, int, f"rpc.services[{si}] '{sname}': clients[{ci}].{key}"):
                if channel_id in used_channels:
                    self.errors.append(LintError(f"rpc.services[{si}] '{sname}': channel collision: {channel_id} ({key})"))
                used_channels.add(channel_id)

    def get_node_ids(self) -> Set[str]:
        """Returns a set of all nodeIds defined in the endpoints."""
        return set(self.endpoint_idx.keys())

    def _check_client_endpoint(self, c: Dict, si: int, ci: int, sname: str):
        ce = c.get("client_endpoint")
        if not self._require_type(ce, dict, f"rpc.services[{si}] '{sname}': clients[{ci}].client_endpoint"):
            return

        cnode = ce.get("nodeId")
        ceid = ce.get("endpointId")
        ctx_node = f"rpc.services[{si}] '{sname}': clients[{ci}].client_endpoint.nodeId"
        ctx_eid = f"rpc.services[{si}] '{sname}': clients[{ci}].client_endpoint.endpointId"

        if not (self._require_type(cnode, str, ctx_node) and cnode): return
        if not (self._require_type(ceid, str, ctx_eid) and ceid): return

        if cnode not in self.endpoint_idx:
            self.errors.append(LintError(f"rpc.services[{si}] '{sname}': client nodeId '{cnode}' not found in rpc.endpoints"))
        elif ceid not in self.endpoint_idx[cnode]:
            self.errors.append(LintError(f"rpc.services[{si}] '{sname}': client endpointId '{ceid}' not found under node '{cnode}'"))
