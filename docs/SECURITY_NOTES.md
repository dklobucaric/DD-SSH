# DD-SSH Security Notes

Initial notes:

- Do not store secrets in logs.
- Do not silently accept changed host keys.
- Real `dd-ssh.json` files with secrets must never be committed.
- Early v1.0.x development uses portable plaintext secrets with `secrets.mode = "plain-v1"`.
- Plaintext password/private-key storage is intentionally insecure and should be used only on trusted machines.
- Sessions reference secrets via `secret_ref` / `key_ref`; the actual values live under top-level `secrets.items`.
- Deleting a saved session must not automatically delete `known_hosts`; host trust is managed separately.
- Future encrypted storage should preserve the session reference structure and only change the `secrets` backend.

## Basic shell channel milestone

Before opening a saved-session shell, DD-SSH must still run the known-host check. Unknown hosts require user trust confirmation, trusted hosts may update `last_seen`, and changed host keys must block shell opening by default.
