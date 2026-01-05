# hakoniwa-remote-api
Transport-agnostic remote API for operating Hakoniwa worlds, assets, and runtimes from external processes.

## Configuration

The behavior of the Hakoniwa Remote API is defined declaratively through a set of JSON configuration files.

### Validation

To ensure the stability and correctness of the configuration, a two-stage validation process is employed:

1.  **Static Schema Validation:** Each JSON file is first validated against its schema using `ajv`. This catches basic structural and data type errors.

2.  **Cross-File Consistency Linter:** After passing schema validation, the `tools/config_lint.py` script is used to perform a deeper analysis. This linter checks for semantic correctness and ensures consistency *between* the different configuration files, including:
    *   Validating that all file path references (e.g., to RPC, PDU, or endpoint configurations) point to existing files.
    *   Ensuring that all `nodeId` and `endpointId` references are resolved correctly.
    *   Detecting collisions and ensuring the uniqueness of names and channel IDs.
    *   Verifying logical constraints, such as client counts not exceeding `maxClients`.

This two-stage approach allows for both robust static verification and the detection of complex, inter-file configuration errors that schemas alone cannot capture.

### Utilities

#### `update_pdusize.py`

このユーティリティは、PDUサイズ定義ファイルに基づいて `rpc.json` 設定ファイル内の `pduSize` フィールドを自動的に更新します。これにより、RPCサービス設定が常に正しいPDUサイズを反映し、手動エラーを防ぎ、更新プロセスを効率化します。

**使用方法:**

```bash
python3 tools/update_pdusize.py <path_to_rpc_json> <path_to_pdu_size_dir>
```

*   `<path_to_rpc_json>`: `rpc.json` ファイルへのパス (例: `config/sample/rpc/rpc.json`)。
*   `<path_to_pdu_size_dir>`: PDUサイズ定義ファイルが配置されているルートディレクトリ (例: `messages/impl/pdu_size`)。

**例:**

サンプル `rpc.json` ファイルを更新する場合:

```bash
python3 tools/update_pdusize.py config/sample/rpc/rpc.json messages/impl/pdu_size
```

