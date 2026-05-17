# DD-SSH Config Format

DD-SSH uses one primary JSON config file:

```text
dd-ssh.json
```

Linux default path:

```text
~/.config/DD-LAB/DD-SSH/dd-ssh.json
```

Windows and macOS use Qt's platform-specific application config location through `QStandardPaths::AppConfigLocation`.

## Top-level structure

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

## Example

```json
{
  "app": {
    "name": "DD-SSH",
    "config_version": 1
  },
  "settings": {
    "appearance": {
      "app_theme": "system"
    },
    "terminal": {
      "font_family": "monospace",
      "font_size": 14
    },
    "config_safety": {
      "backups_enabled": true,
      "max_backups": 10
    },
    "behavior": {
      "double_click_action": "open_terminal",
      "show_quick_toolbar": false
    }
  },
  "groups": [],
  "sessions": [
    {
      "id": "root-192-168-1-237-223",
      "name": "Local test server",
      "group": "Lab",
      "host": "192.168.1.237",
      "port": 223,
      "username": "root",
      "auth": {
        "type": "password",
        "secret_ref": "secret-root-192-168-1-237-223-password"
      }
    }
  ],
  "known_hosts": {
    "192.168.1.237:223": {
      "algorithm": "ssh-ed25519",
      "fingerprint": "SHA256:example",
      "first_seen": "2026-05-17T12:00:00Z",
      "last_seen": "2026-05-17T12:30:00Z"
    }
  },
  "secrets": {
    "mode": "plain-v1",
    "items": {
      "secret-root-192-168-1-237-223-password": {
        "type": "password",
        "value": "plaintext-password"
      }
    }
  },
  "metadata": {
    "created_by": "DD-SSH"
  }
}
```

## Settings

### Appearance

```json
"appearance": {
  "app_theme": "system"
}
```

Supported values:

```text
system
light
dark
```

This currently affects the Qt application chrome only. The xterm.js terminal theme is intentionally unchanged.

### Terminal

```json
"terminal": {
  "font_family": "monospace",
  "font_size": 14
}
```

Terminal font settings apply to newly opened xterm.js tabs.

### Config safety

```json
"config_safety": {
  "backups_enabled": true,
  "max_backups": 10
}
```

When enabled, DD-SSH creates rotating `dd-ssh.json.bak-*` backups before saves.

### Behavior

```json
"behavior": {
  "double_click_action": "open_terminal",
  "show_quick_toolbar": false
}
```

`double_click_action` is currently expected to be `open_terminal`.

## Sessions

A session describes a connection target and auth method.

Password session:

```json
{
  "id": "server-1",
  "name": "Server 1",
  "group": "Lab",
  "host": "192.168.1.237",
  "port": 223,
  "username": "root",
  "auth": {
    "type": "password",
    "secret_ref": "secret-server-1-password"
  }
}
```

Private-key session:

```json
{
  "id": "server-key",
  "name": "Server Key",
  "group": "Lab",
  "host": "192.168.1.222",
  "port": 223,
  "username": "root",
  "auth": {
    "type": "key",
    "key_ref": "secret-server-key"
  }
}
```

## Secrets

Early DD-SSH builds use:

```json
"secrets": {
  "mode": "plain-v1",
  "items": {}
}
```

Password secret:

```json
"secret-server-password": {
  "type": "password",
  "value": "plaintext-password"
}
```

Private-key secret:

```json
"secret-server-key": {
  "type": "private_key",
  "value": "-----BEGIN OPENSSH PRIVATE KEY-----\n...\n-----END OPENSSH PRIVATE KEY-----\n"
}
```

Sessions reference secrets. Secrets are not stored directly inside the session object.

This is intentional because future encrypted storage can replace the `secrets` backend without changing saved session structure.

## known_hosts

Known-host records are stored separately from sessions.

Deleting a session does not automatically delete `known_hosts`.

Reason: several sessions may point to the same host/port with different usernames or auth methods.

## Import/export behavior

Import/export operates on the whole config file.

Import replaces:

```text
sessions
secrets
known_hosts
settings
metadata
```

Before import, DD-SSH creates a pre-import backup of the active config.

## Corrupt config behavior

If `dd-ssh.json` is invalid JSON or not a JSON object, DD-SSH refuses to overwrite it automatically.

Recovery options:

```text
Continue read-only
Restore latest valid backup
Create fresh config
Open config folder
```

Corrupt files are preserved as:

```text
dd-ssh.json.corrupt-<timestamp>
```
