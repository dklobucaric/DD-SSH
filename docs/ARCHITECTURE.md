# DD-SSH Architecture

DD-SSH is built as a small layered Qt/C++ application.

## High-level layers

```text
+------------------------------------------------------------+
| UI Layer                                                   |
| MainWindow, ConnectDialog, SettingsDialog, WebTerminalTab  |
+------------------------------------------------------------+
| Core Layer                                                 |
| ConfigManager, KnownHostsManager, SessionProfile           |
+------------------------------------------------------------+
| SSH Layer                                                  |
| SshSession, SshShellWorker                                 |
+------------------------------------------------------------+
| Terminal Layer                                             |
| xterm.js in Qt WebEngine, TerminalBridge                   |
+------------------------------------------------------------+
| Config File                                                |
| dd-ssh.json                                                |
+------------------------------------------------------------+
```

## Main UI classes

### MainWindow

Responsibilities:

- Main window layout
- Menus
- Optional quick toolbar
- Sidebar session list
- Terminal tabs
- File config actions
- Session actions
- Settings dialog entry
- About dialog
- Config recovery warning

MainWindow coordinates behavior but should not contain low-level SSH protocol details.

### ConnectDialog

Shared dialog with explicit modes:

```text
Manual Connect
New Saved Session
Edit Saved Session
```

### SettingsDialog

Edits settings stored under `settings` in `dd-ssh.json`:

- Appearance
- Terminal font
- Config backup policy
- Quick toolbar visibility

### WebTerminalTab

xterm.js terminal tab.

Responsibilities:

- Load local xterm.js resources
- Own terminal UI controls
- Forward keyboard/paste/Ctrl+C to SSH worker
- Receive output and write to xterm.js
- Track connected/disconnected state
- Reconnect using same saved session
- Handle PTY resize notifications

### BasicTerminalTab

Temporary QWidget input/output fallback used for debugging.

## Core classes

### ConfigManager

Responsibilities:

- Locate config file
- Load/save JSON
- Load/save sessions
- Load/save settings
- Manage plaintext secrets
- Export/import config
- Restore latest backup
- Create rotating backups
- Inspect corrupt config
- Create fresh default config

### KnownHostsManager

Responsibilities:

- Store known-host trust records in `dd-ssh.json`
- Compare host key fingerprints
- Trust unknown hosts when user confirms
- Refuse changed-host silent acceptance
- Avoid overwriting invalid config

### SessionProfile

Value object representing one saved session:

```text
id
name
group
host
port
username
auth type
secret_ref/key_ref
```

## SSH classes

### SshSession

Manual/auth-test style wrapper around libssh connection/auth logic.

### SshShellWorker

Runs an interactive SSH shell channel in a worker thread.

Responsibilities:

- Connect
- Known-host/auth support through supplied values
- Open PTY
- Open shell
- Read remote output
- Write queued input
- Resize PTY
- Disconnect cleanly
- Emit lifecycle signals

## Terminal bridge

### TerminalBridge

Qt WebChannel bridge between JavaScript/xterm.js and C++.

Used for:

- Input from browser terminal to C++
- Resize notifications from xterm.js to C++

## Threading model

```text
GUI thread:
- Main window
- Settings
- Menus
- Sidebar
- Qt WebEngine widget
- xterm.js rendering

Worker thread:
- SSH shell connect/auth/read/write loop
- PTY resize handling
- disconnect cleanup
```

The SSH read loop must not block the GUI thread.

## Terminal data flow

User input:

```text
xterm.js onData()
→ TerminalBridge
→ WebTerminalTab
→ SshShellWorker input queue
→ libssh channel write
→ remote shell
```

Remote output:

```text
remote shell
→ libssh channel read
→ SshShellWorker output signal
→ WebTerminalTab
→ xterm.write(data)
```

PTY resize:

```text
Qt/WebEngine size change
→ xterm FitAddon
→ cols/rows
→ TerminalBridge terminalResized()
→ SshShellWorker resizePty()
→ ssh_channel_change_pty_size()
```

## Config safety design

Config writes should be safe:

- Use backups when enabled.
- Do not overwrite invalid JSON automatically.
- Preserve corrupt configs as `.corrupt-*` when creating fresh config.
- Create pre-import/pre-restore backups before replacing active config.

## Future architecture direction

Likely future classes:

```text
SessionManager
MultiExecManager
KeepAliveManager
ConfigPathManager
ThemeManager
```

Do not add large responsibilities to MainWindow when a focused class is appropriate.
