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

Secrets may be plain during early dev-only testing, but the structure should allow encrypted secrets later.
