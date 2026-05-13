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

When saving a new session, DD-SSH should warn if another session already uses the same `username + host + port` target. The user should be able to update the existing session, create a copy, or cancel.


## Saved session shell channel

`dev 0.1.2.0` can open an experimental shell channel from a saved session. `dev 0.1.2.1` cleans up the temporary shell UI. `dev 0.1.2.3` adds a Qt WebEngine terminal fallback with paste support as preparation for xterm.js. `dev 0.1.2.4` fixes direct input/paste dispatch in that fallback terminal. `dev 0.1.2.5` adds focus polish for the fallback terminal. `dev 0.1.3.0` introduced the first xterm.js renderer path while keeping the fallback renderer if xterm.js assets cannot be loaded. `dev 0.1.3.1` adds xterm FitAddon plus SSH PTY resize sync. These shell/frontend milestones do not change the JSON format. The session still resolves authentication through `auth.secret_ref` or `auth.key_ref` and loads the plaintext value from `secrets.items` when `secrets.mode` is `plain-v1`.
