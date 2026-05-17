# DD-SSH User Guide

## Main window

DD-SSH uses a simple layout:

```text
Left side: saved sessions from dd-ssh.json
Right side: terminal tabs
Bottom: status bar
```

Double-click a saved session to open the default xterm.js terminal.

## Menus

### File

Config-level and app-level actions.

```text
Open Config Folder
Export Config...
Import Config...
Restore Latest Backup...
Exit
```

### Session

SSH session workflow actions.

```text
New Session
Connect / Auth test
Edit selected session
```

### Tools

Utility and app configuration actions.

```text
Multi-Exec (placeholder for future work)
Settings
```

### Help

```text
About DD-SSH
```

## Session workflows

### New Session

Creates a saved session after successful authentication.

Use this when adding a server you want to keep.

### Connect / Auth test

Runs a manual connection/authentication test. Saving is optional.

Use this when testing credentials, host keys, ports, or temporary access.

### Edit selected session

Edits the currently selected sidebar session.

For password/private-key secrets:

```text
Leave password/key empty to keep the existing saved secret.
Enter a new password/key to replace the saved secret.
```

## Sidebar context menu

Right-click a saved session:

```text
Open xterm.js terminal
Run auth test
Open basic shell fallback
Edit session
Delete session
```

The basic shell fallback is a debug/development fallback. The normal terminal is xterm.js.

## Terminal tabs

The xterm.js terminal supports:

- Direct keyboard input
- Paste
- Ctrl+C
- PTY resize sync
- Full-screen terminal apps such as `htop`, `nano`, `vim`, `top`
- Reconnect after disconnect

Terminal buttons:

```text
Ctrl+C     Send interrupt to remote shell
Paste      Send clipboard text to terminal
Clear      Clear local terminal view
Reset      Reset local xterm renderer state
Focus      Focus terminal area
Reconnect  Reconnect after disconnect
Disconnect Disconnect SSH shell
```

## Terminal lifecycle

When the remote shell exits, server reboots, or SSH transport drops, DD-SSH marks the tab disconnected and disables remote input actions.

After disconnect, use:

```text
Reconnect
```

The reconnect action uses the same saved session and plaintext secret.

## Settings

Open:

```text
Tools → Settings
```

### General

- Config path display
- Open config folder
- Double-click session action display
- Show quick action toolbar

### Appearance

- System default
- Light
- Dark

This changes the Qt application chrome only. The xterm.js terminal theme remains dark for now.

### Terminal

- Font family
- Font size

Font settings apply to newly opened xterm.js terminal tabs. Existing terminal tabs are not changed live yet.

### Config safety

- Enable config backups before save
- Keep last N backups

## Config import/export

### Export Config

Copies the active `dd-ssh.json` to a user-selected path.

Warning: exported configs may contain plaintext passwords and private keys.

### Import Config

Validates a selected JSON file, warns, creates a backup of the current active config, then replaces the active config.

Import replaces:

```text
sessions
secrets
known_hosts
settings
metadata
```

### Restore Latest Backup

Restores the newest valid `dd-ssh.json.bak-*` file from the config folder.

The previous active config is moved aside as:

```text
dd-ssh.json.pre-restore-<timestamp>
```
