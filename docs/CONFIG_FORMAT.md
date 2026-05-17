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

`dev 0.1.2.0` can open an experimental shell channel from a saved session. `dev 0.1.2.1` cleans up the temporary shell UI. `dev 0.1.2.3` adds a Qt WebEngine terminal fallback with paste support as preparation for xterm.js. `dev 0.1.2.4` fixes direct input/paste dispatch in that fallback terminal. `dev 0.1.2.5` adds focus polish for the fallback terminal. `dev 0.1.3.0` introduced the first xterm.js renderer path while keeping the fallback renderer if xterm.js assets cannot be loaded. `dev 0.1.3.1` adds xterm FitAddon plus SSH PTY resize sync. `dev 0.1.3.2.2` fixes the local xterm.js/FitAddon Qt resource paths for offline runtime use. `dev 0.1.3.3` adds the Andromeda terminal compatibility polish and still does not change the JSON format. The session still resolves authentication through `auth.secret_ref` or `auth.key_ref` and loads the plaintext value from `secrets.items` when `secrets.mode` is `plain-v1`.

## Settings block

`dev 0.1.4.0` introduces the first Settings foundation. The settings block is intentionally small and safe:

```json
{
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
  }
}
```

App theme values are `system`, `light`, or `dark`. The app theme affects the Qt application chrome only; xterm.js terminal colors are intentionally not changed in this checkpoint. Terminal font settings apply to newly opened xterm.js terminal tabs. Starting with `dev 0.1.4.2`, the config safety backup policy is active: when backups are enabled, DD-SSH creates timestamped `dd-ssh.json.bak-*` files before saving and keeps the newest `max_backups` files. Starting with `dev 0.1.4.7`, `settings.behavior.show_quick_toolbar` controls whether the optional quick action toolbar is visible. It defaults to `false` so the main window stays menu-driven and cleaner for public-alpha testing. Starting with `dev 0.1.4.8`, the File menu can export the active `dd-ssh.json`, import a validated replacement config with a pre-import backup, and restore the latest valid `dd-ssh.json.bak-*` backup.

Default config location notes:

```text
Linux:   ~/.config/DD-LAB/DD-SSH/dd-ssh.json
Windows: Qt AppConfigLocation / per-user AppData-style config folder
macOS:   Qt AppConfigLocation / per-user Application Support-style config folder
```

The exact Windows/macOS base directory is selected by Qt through `QStandardPaths::AppConfigLocation`; DD-SSH does not write next to the executable unless a future portable-mode feature is added.

## Config recovery and backups

`dev 0.1.4.5` adds a safety guard for damaged config files. If `dd-ssh.json` exists but cannot be parsed as a JSON object, DD-SSH must not overwrite it automatically. The app may continue with default in-memory settings and an empty session list, but save operations should fail until the user fixes, restores, moves, or deletes the broken file.

`dev 0.1.4.5.1` adds explicit recovery actions. From the recovery dialog, the user can continue read-only, open the config folder, restore the latest valid `dd-ssh.json.bak-*` backup, or create a fresh empty config. Creating a fresh config and restoring a backup both move the corrupt file aside as `dd-ssh.json.corrupt-<timestamp>` instead of deleting it.

When config backups are enabled under `settings.config_safety`, DD-SSH creates timestamped files named like:

```text
dd-ssh.json.bak-YYYYMMDD-HHMMSS-zzz
```

The recovery warning lists available `dd-ssh.json.bak-*` files in the config folder so the user can restore one manually. Automatic restore UI is intentionally deferred.

## Session workflow note — dev 0.1.4.6+

`dev 0.1.4.6` clarifies UI behavior around the existing session/config model. Manual Connect may optionally save a session after successful authentication, while New Session always creates or updates a saved session after successful authentication. Edit Session updates an existing `sessions[]` entry and keeps the existing plaintext secret when password/key fields are left empty.

`dev 0.1.4.7` adds the optional `settings.behavior.show_quick_toolbar` value. It does not affect saved sessions, secrets, or known-host records; it only controls whether the shortcut toolbar is shown in the main window.

## Config import/export behavior

`dev 0.1.4.8` adds File-menu config import/export/restore actions. These operate on the complete `dd-ssh.json` file, not just visual settings. That means exported/imported configs include sessions, known-host records, plaintext secrets, settings, and metadata.

Import safety rules:

```text
- selected import file must be valid JSON
- root JSON value must be an object
- current config must not be corrupt; use recovery first if it is
- current config is copied to dd-ssh.json.bak-import-* before replacement
- imported config is normalized with the base app/settings/sessions/secrets objects
```

Restore behavior:

```text
- Restore Latest Backup picks the newest valid dd-ssh.json.bak-* file
- the active config is moved aside as dd-ssh.json.pre-restore-*
- the selected backup becomes the active dd-ssh.json
```

Security reminder: exported configs may contain plaintext passwords and private keys when `secrets.mode = "plain-v1"`.
