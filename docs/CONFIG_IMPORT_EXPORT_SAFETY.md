# Config Import/Export Safety — dev 0.1.6.8

**Checkpoint:** dev 0.1.6.8 — Andromeda

DD-SSH intentionally keeps `dd-ssh.json` as a human-readable JSON file during the early 0.x line. This checkpoint does not encrypt or hide the config. It adds safer visibility before import/export actions.

## Import preview

Before replacing the active config, DD-SSH now previews the selected JSON file:

```text
Import file: /path/to/dd-ssh.json
File size: 42.1 KB
Valid JSON object: YES
Config version: 1
Sessions: 13
Known hosts: 8 host(s), 12 trusted key(s)
Secrets mode: plain-v1
Saved secrets: 13 total (8 password, 5 private key)
Contains plaintext secret values: YES
Settings section: present
Metadata section: present
```

The import still creates a pre-import backup before replacing the active config.

## Export preview

Before export, DD-SSH previews the active config and warns that exported JSON may contain saved sessions, known hosts, settings, and plaintext `plain-v1` secrets.

## What this does not do

`dev 0.1.6.8` does not add encryption, a master password, OS keychain storage, or `Export without secrets`. Those can be planned later without breaking the current human-readable JSON format.

## Logging

If diagnostic logging is enabled, preview operations log only counts and flags such as session count, known-host count, secrets mode, and plaintext-secret presence. Logs must not contain password values, private-key contents, terminal input/output, clipboard content, or full config JSON.
