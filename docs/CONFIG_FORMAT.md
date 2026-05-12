# DD-SSH Config Format

Primary config file:

```text
dd-ssh.json
```

Top-level structure:

```json
{
  "app": {},
  "settings": {},
  "groups": [],
  "sessions": [],
  "known_hosts": {},
  "secrets": {},
  "metadata": {}
}
```

The config file should be portable, human-readable, versioned, and sync-friendly.

For early v1.0.x development, DD-SSH may store passwords and private keys in plaintext under:

```json
{
  "secrets": {
    "mode": "plain-v1",
    "items": {}
  }
}
```

Sessions should reference secrets with `secret_ref` or `key_ref`; password/private-key values should live under the top-level `secrets.items` object. This keeps the format portable now and leaves room for encrypted secrets later without breaking saved session structure.

Deleting a saved session should not automatically delete `known_hosts`. Known-host trust is a separate security record.

