# DD-SSH Security Notes

DD-SSH is currently an early public-alpha candidate. Security decisions are explicit and must remain visible to users.

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
- Trusted host can continue when fingerprint matches.
- Changed host key must not be silently accepted.
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
