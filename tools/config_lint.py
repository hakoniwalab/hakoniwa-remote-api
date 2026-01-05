#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Main entry point for the Hakoniwa config linker/linter.
"""

from __future__ import annotations

import argparse
import os
from typing import List

from linter.base import LintError
from linter.remote_api_linter import RemoteApiLinter
from linter.rpc_linter import RpcLinter


def main() -> int:
    """Main function to run the linters."""
    ap = argparse.ArgumentParser(description="Hakoniwa config cross-file linter (no deps)")
    ap.add_argument("remote_api_json", help="path to remote-api.json")
    args = ap.parse_args()

    all_errors: List[LintError] = []
    rpc_linter = None

    # Lint remote-api.json
    remote_linter = RemoteApiLinter(args.remote_api_json)
    all_errors.extend(remote_linter.lint())

    # If remote-api has data, try to lint rpc.json
    if remote_linter.data:
        rpc_rel = remote_linter.get_rpc_path()
        if isinstance(rpc_rel, str) and rpc_rel:
            base_dir = os.path.dirname(os.path.abspath(args.remote_api_json))
            rpc_path = os.path.normpath(os.path.join(base_dir, rpc_rel))
            
            if os.path.exists(rpc_path):
                rpc_linter = RpcLinter(rpc_path)
                all_errors.extend(rpc_linter.lint())
            else:
                 all_errors.append(LintError(
                    f"remote-api.rpc_service_config_path: not found: '{rpc_rel}' (resolved: '{rpc_path}')"
                ))

    # Perform cross-file checks if both files were loaded successfully
    if remote_linter.data and rpc_linter and rpc_linter.data:
        all_errors.extend(
            _check_cross_file_references(remote_linter, rpc_linter)
        )

    if all_errors:
        print("LINT FAILED:")
        for e in all_errors:
            print(f" - {e}")
        return 1

    print("LINT OK")
    return 0

def _check_cross_file_references(remote_linter: RemoteApiLinter, rpc_linter: RpcLinter) -> List[LintError]:
    """
    Checks for validity of references between remote-api.json and rpc.json.
    """
    errors: List[LintError] = []
    
    # Check if participant nodeId from remote-api.json exists in rpc.json
    rpc_node_ids = rpc_linter.get_node_ids()
    participants = remote_linter.get_participants()

    for i, p in enumerate(participants):
        node_id = p.get("nodeId")
        if isinstance(node_id, str) and node_id:
            if node_id not in rpc_node_ids:
                errors.append(LintError(
                    f"remote-api.participants[{i}]: nodeId '{node_id}' not found in rpc.json's endpoints"
                ))
    
    return errors


if __name__ == "__main__":
    raise SystemExit(main())