# DD-SSH Security Notes

DD-SSH is currently an early public-alpha candidate. Security decisions are explicit and must remain visible to users.

## SSH host-key trust chain

Starting with `dev 0.1.6.3`, DD-SSH must verify the SSH host key in the same connection that performs authentication or opens the shell.

Required rule:

```text
ssh_connect()
read current server host key
compare type + SHA256 fingerprint with the preflight-approved key
only then send password/private key authentication
```

If the real authentication/shell connection reports a different key than the key approved during preflight, DD-SSH must abort before authentication and tell the user that authentication was not attempted.

This rule must apply to:

- Saved-session xterm.js terminal opens
- Saved-session basic shell fallback opens
- Saved-session authentication tests
- Manual connection/authentication tests

The multi-key known-host model remains valid: one `host:port` may store more than one legitimate key algorithm/fingerprint pair, for example ED25519 on Linux/Windows 11 and ECDSA on Windows 10. The active connection must still match the key approved for that attempt before auth is sent.

## Plaintext secrets

Current early builds use:

```json
"secrets": {
  "mode": "plain-v1"
}
```

This means saved passwords and private keys may be stored in plaintext inside `dd-ssh.json`.

This is intentional for early portability, but it is insecure on untrusted machines.

## User warning

The Settings dialog displays a plaintext warning. Keep it visible.

Recommended wording:

```text
DD-SSH currently uses secrets.mode = plain-v1. Saved passwords and private keys are portable but stored in plaintext in dd-ssh.json.
```

## Never log secrets

Do not print:

- Password values
- Private key contents
- Full `dd-ssh.json` if it contains secrets
- Clipboard contents sent to terminal

Auth test output should say:

```text
Saved password: loaded from JSON, hidden from display
Saved private key: loaded from JSON, hidden from display
```

## known_hosts

Known-host verification is mandatory.

Rules:

- Unknown host must require user trust confirmation.
- Trusted host can continue when the current key type and fingerprint match a saved key.
- A new legitimate host-key algorithm for the same `host:port` must be treated as an additional key, not as an automatic replacement.
- Changed same-key-type fingerprint must not be silently accepted.
- Deleting a saved session must not automatically delete `known_hosts`.

Why not delete known_hosts with sessions?

Because multiple sessions can point to the same host/port with different usernames or auth methods.

## Config backups

Config backups may also contain plaintext secrets.

Backups are useful, but they are sensitive files.

Treat these as secret-bearing files:

```text
dd-ssh.json
dd-ssh.json.bak-*
dd-ssh.json.pre-import-*
dd-ssh.json.pre-restore-*
dd-ssh.json.corrupt-*
```

## Exported config

Exported config may contain passwords and private keys.

Do not share exported config publicly.

## Git safety

Real config files must not be committed.

Recommended `.gitignore` patterns:

```gitignore
dd-ssh.json
dd-ssh.json.*
*.pem
*.key
id_rsa
id_ed25519
```

## Future encrypted secrets

Future versions may support:

```text
secrets.mode = encrypted
master password
key derivation
AES-GCM or similar authenticated encryption
```

Important principle:

```text
Sessions should keep referencing secret_ref/key_ref.
Only the secrets backend should change.
```

## Multi-Exec safety future note

Multi-Exec will be powerful and dangerous.

It must include:

- Target preview
- Send text only vs send + Enter
- Dangerous command warning
- Safe default action
- No hidden execution
- Local log without secrets


## Diagnostic logging secret-safety

`dev 0.1.6.6` adds optional diagnostic logging. Logging is OFF by default and is intended for on-demand troubleshooting only.

Logs must never include passwords, private-key contents, plaintext secret values, terminal input, terminal output, clipboard contents, or the full `dd-ssh.json` file. Logs may include troubleshooting metadata such as host, port, username, authentication method, host-key type, and host-key fingerprint.

Standard log folders:

```text
Linux: ~/.local/state/DD-SSH/logs
Windows: %LOCALAPPDATA%\DD-SSH\logs
macOS: ~/Library/Logs/DD-SSH
```
