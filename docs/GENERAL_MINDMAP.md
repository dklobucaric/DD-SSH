# DD-SSH Project Blueprint v0.1

**Project name:** DD-SSH  
**Project type:** Cross-platform SSH client / session manager  
**Primary platforms:** Linux, Windows, macOS  
**Project status:** Planning / architecture phase  
**Primary goal:** Build a clean, lightweight, practical SSH client inspired by the simplicity of PortX, but focused only on the features that matter for everyday sysadmin work.

---

## 1. Executive Summary

DD-SSH is intended to be a cross-platform SSH terminal client and session manager built for users who manage multiple servers and want a simple, fast, reliable tool without enterprise bloat, cloud lock-in, SFTP panels, split screens, telemetry, or unnecessary visual noise.

The application should run on Linux, Windows, and macOS from a shared codebase. It should eventually be distributed through multiple channels: source code on GitHub, Linux packages, Windows installer, macOS app bundle/DMG, and potentially official stores where feasible.

The project should be built carefully and incrementally. The first major technical milestone is not packaging or store upload. The first milestone is a stable SSH terminal proof-of-concept.

The core idea:

```text
One clean SSH app.
One consistent config format.
One codebase.
Multiple operating systems.
No nonsense.
```

---

## 2. Product Philosophy

DD-SSH should feel like a practical sysadmin tool, not a spaceship dashboard with 600 buttons.

The philosophy:

```text
Fast to open.
Fast to connect.
Easy to understand.
Easy to sync.
Easy to back up.
Hard to accidentally destroy things.
```

The user should be able to:

1. Open DD-SSH.
2. See a clean list of saved connections.
3. Double-click a server.
4. Get a real SSH terminal.
5. Work.

DD-SSH should avoid becoming a clone of large commercial terminal suites. It should instead focus on doing a smaller set of things very well.

---

## 3. What DD-SSH Is

DD-SSH is:

```text
- A cross-platform desktop SSH client
- A terminal emulator frontend connected to real SSH sessions
- A session manager for many servers
- A JSON-config-driven portable tool
- A practical sysadmin utility
- An open-source project
- A product that can eventually be packaged and distributed properly
```

DD-SSH is not:

```text
- A full remote management platform
- A cloud service
- A team collaboration suite
- A replacement for Ansible
- A file manager
- An SFTP client in v1
- A split-screen terminal workspace in v1
- An AI-controlled server operator
```

---

## 4. Core Feature Scope

The initial desired feature list is:

```text
1. SSH terminal
2. Session manager
3. Password authentication
4. Private-key authentication
5. Tabs
6. Copy/paste
7. Export/import config/session data
8. Sync-friendly connection config
9. Keep-alive
10. Known-host handling
11. Multi-exec command
```

Explicitly excluded from early versions:

```text
- SFTP
- Split screen
- Built-in cloud account
- Built-in WebDAV sync in v1
- Android version
- Team sharing
- AI agent inside the application
- Automatic remote command execution without user confirmation
```

---

## 5. Priority Order

The project must not start with packaging, stores, icons, or fancy UI.

The correct priority is:

```text
1. SSH terminal proof-of-concept
2. Stable SSH backend abstraction
3. Terminal frontend abstraction
4. Basic connection dialog
5. Known-host handling
6. Password authentication
7. Private-key authentication
8. Session manager
9. Single JSON config
10. Tabs
11. Keep-alive
12. Export/import
13. Sync-friendly config path
14. Multi-exec
15. Packaging
16. Store distribution
```

The rule:

```text
Until one SSH terminal tab works reliably, everything else is secondary.
```

---

## 6. Technology Direction

### 6.1 Primary Stack

Recommended initial stack:

```text
Language: C++
GUI framework: Qt 6
Build system: CMake
SSH library: libssh
Terminal frontend: xterm.js inside Qt WebEngine, at least for the first proof-of-concept
Config format: JSON
Version control: Git / GitHub
```

### 6.2 Why Qt 6

Qt gives us:

```text
- Cross-platform GUI support
- Native desktop app structure
- Good CMake integration
- Linux/Windows/macOS support
- Mature widgets
- Packaging/deployment tooling
```

### 6.3 Why CMake

CMake gives us:

```text
- Cross-platform build configuration
- Compatibility with Qt Creator
- Compatibility with CI/CD pipelines
- Linux/Windows/macOS build definitions
- Future GitHub Actions support
```

### 6.4 Why libssh

libssh gives us:

```text
- SSHv2 client functionality
- Password authentication
- Public/private key authentication
- Channel handling
- PTY/shell support
- Known-host functionality or host key access
- Keep-alive support or keep-alive-friendly session behavior
```

### 6.5 Why xterm.js Initially

Terminal emulation is one of the hardest parts of the project.

A real terminal must handle:

```text
- ANSI escape sequences
- Cursor movement
- Colors
- UTF-8
- Resize
- Scrollback
- Copy/paste
- Interactive programs like vim, nano, htop, mc, tmux
```

Using xterm.js gives us a mature terminal frontend quickly.

The tradeoff:

```text
Pros:
- Faster proof-of-concept
- Good terminal behavior
- Cross-platform rendering through WebEngine

Cons:
- Larger application package
- Qt WebEngine dependency
- More complex packaging
```

Long-term, the architecture should allow the terminal frontend to be replaced later with a native Qt/libvterm-based renderer if desired.

---

## 7. High-Level Architecture

```text
+------------------------------------------------------------+
|                          DD-SSH                            |
+------------------------------------------------------------+
|                                                            |
|  UI Layer                                                  |
|  - MainWindow                                              |
|  - Session sidebar                                         |
|  - Terminal tabs                                           |
|  - Settings dialog                                         |
|  - Multi-exec panel                                        |
|                                                            |
+------------------------------------------------------------+
|                                                            |
|  Core Layer                                                |
|  - SessionManager                                          |
|  - ConfigManager                                           |
|  - KnownHostsManager                                       |
|  - MultiExecManager                                        |
|  - SyncManager                                             |
|                                                            |
+------------------------------------------------------------+
|                                                            |
|  SSH Layer                                                 |
|  - SshSession                                              |
|  - SshWorker                                               |
|  - AuthManager                                             |
|  - KeepAliveManager                                        |
|                                                            |
+------------------------------------------------------------+
|                                                            |
|  Terminal Layer                                            |
|  - TerminalTab                                             |
|  - TerminalFrontend interface                              |
|  - XtermJsTerminalFrontend                                 |
|  - FutureNativeTerminalFrontend                            |
|                                                            |
+------------------------------------------------------------+
```

The application should be modular from the beginning. Even if the first version is small, the internal design should not be one giant source file.

---

## 8. Core Data Flow

### 8.1 User Input to Server

```text
User keyboard input
        |
        v
Terminal frontend
        |
        v
TerminalTab
        |
        v
SshWorker / SshSession
        |
        v
libssh channel write
        |
        v
Remote server shell
```

### 8.2 Server Output to User

```text
Remote server shell
        |
        v
libssh channel read
        |
        v
SshWorker emits output signal
        |
        v
TerminalTab receives output
        |
        v
Terminal frontend writes output
        |
        v
User sees terminal screen
```

### 8.3 Threading Rule

SSH connection and read loops must not block the GUI thread.

```text
GUI thread:
- Main window
- Tabs
- Session list
- Terminal rendering
- User interaction

Worker thread:
- SSH connect
- SSH authentication
- SSH channel read/write
- Keep-alive
```

If the SSH read loop blocks the UI thread, the whole application will freeze.

---

## 9. Proposed Source Tree

Initial project layout:

```text
DD-SSH/
  README.md
  LICENSE
  AGENTS.md
  CMakeLists.txt

  docs/
    ARCHITECTURE.md
    ROADMAP.md
    CONFIG_FORMAT.md
    BUILDING.md
    PACKAGING.md
    SECURITY_NOTES.md
    RELEASE_PROCESS.md

  src/
    main.cpp

    ui/
      MainWindow.h
      MainWindow.cpp
      ConnectDialog.h
      ConnectDialog.cpp
      SessionSidebar.h
      SessionSidebar.cpp
      TerminalTab.h
      TerminalTab.cpp
      MultiExecPanel.h
      MultiExecPanel.cpp

    core/
      AppContext.h
      AppContext.cpp
      SessionProfile.h
      SessionManager.h
      SessionManager.cpp
      ConfigManager.h
      ConfigManager.cpp
      KnownHostsManager.h
      KnownHostsManager.cpp
      MultiExecManager.h
      MultiExecManager.cpp
      SyncManager.h
      SyncManager.cpp

    ssh/
      SshSession.h
      SshSession.cpp
      SshWorker.h
      SshWorker.cpp
      AuthManager.h
      AuthManager.cpp
      KeepAliveManager.h
      KeepAliveManager.cpp

    terminal/
      TerminalFrontend.h
      XtermJsTerminalFrontend.h
      XtermJsTerminalFrontend.cpp
      NativeTerminalFrontend.h
      NativeTerminalFrontend.cpp

  resources/
    xterm/
      terminal.html
      xterm.js
      xterm.css

    icons/
      dd-ssh.svg
      dd-ssh.png

  examples/
    dd-ssh.example.json

  tests/
    CMakeLists.txt
    test_config_manager.cpp
    test_session_manager.cpp
    test_known_hosts_manager.cpp
```

This structure can evolve, but it gives us clean boundaries immediately.

---

## 10. Main Internal Components

### 10.1 MainWindow

Responsible for:

```text
- Main application window
- Menu bar / toolbar
- Session sidebar
- Terminal tab container
- Opening dialogs
- Coordinating high-level UI actions
```

MainWindow should not contain SSH protocol logic.

### 10.2 SessionProfile

Represents one saved connection.

Fields may include:

```text
- id
- name
- group
- host
- port
- username
- auth type
- password reference
- key reference
- keep-alive settings
- terminal settings
- tags
- notes
```

### 10.3 SessionManager

Responsible for:

```text
- Holding all sessions in memory
- Searching/filtering sessions
- Grouping sessions
- Creating/editing/deleting sessions
- Exposing session data to UI
```

### 10.4 ConfigManager

Responsible for:

```text
- Loading dd-ssh.json
- Saving dd-ssh.json
- Validating config version
- Creating backups before saving
- Import/export logic
- Handling config format migrations later
```

### 10.5 KnownHostsManager

Responsible for:

```text
- Storing trusted host fingerprints
- Detecting unknown hosts
- Detecting changed host keys
- Asking the UI layer for user confirmation
- Updating known-host records
```

### 10.6 SshSession

Wrapper around libssh session/channel logic.

Responsible for:

```text
- Creating SSH session
- Setting host/port/user
- Connecting
- Authenticating
- Opening PTY
- Opening shell
- Writing data
- Reading data
- Resizing PTY
- Disconnecting
```

### 10.7 SshWorker

Runs SSH operations outside the GUI thread.

Responsible for:

```text
- Running connect/auth/read loop in background
- Emitting signals when output arrives
- Emitting state changes
- Receiving input from terminal tab
- Cleanly stopping sessions
```

### 10.8 TerminalFrontend

Abstract interface between app logic and terminal rendering.

Required methods conceptually:

```text
writeOutput(data)
sendInput(data)
resize(cols, rows)
copy()
paste()
clear()
focus()
```

The application should not be tightly coupled to xterm.js.

### 10.9 XtermJsTerminalFrontend

Initial terminal implementation using:

```text
Qt WebEngine + local terminal.html + xterm.js
```

Responsible for:

```text
- Rendering terminal screen
- Capturing user input
- Sending input to C++ bridge
- Receiving output from SSH layer
- Handling terminal resize
```

### 10.10 MultiExecManager

Responsible for:

```text
- Selecting target sessions/tabs
- Preparing command preview
- Sending text only or text + Enter
- Logging multi-exec actions locally
- Warning about dangerous commands
```

---

## 11. Connection Flow

When the user connects to a server:

```text
1. User selects a session or opens manual connect dialog.
2. App loads SessionProfile.
3. TerminalTab is created.
4. SshWorker starts in worker thread.
5. libssh connects to host:port.
6. Host key is checked by KnownHostsManager.
7. User confirms unknown host if needed.
8. Authentication is performed.
9. PTY is requested.
10. Shell is opened.
11. Terminal output starts flowing to TerminalFrontend.
12. User input flows back to SSH channel.
```

Diagram:

```text
[User selects session]
          |
          v
[Load SessionProfile]
          |
          v
[Create TerminalTab]
          |
          v
[Start SshWorker]
          |
          v
[SSH handshake]
          |
          v
[Known-host check]
          |
          v
[Authentication]
          |
          v
[Open PTY + shell]
          |
          v
[Interactive terminal]
```

---

## 12. Single JSON Config Design

The project should be designed around one primary config file:

```text
dd-ssh.json
```

This file should be portable, sync-friendly, human-readable, and versioned.

### 12.1 Why One JSON File

Benefits:

```text
- Easy backup
- Easy export/import
- Easy sync with Nextcloud/WebDAV/Syncthing
- Easy manual inspection
- Easy Git-based versioning if user wants it
- Consistent across OSes
```

### 12.2 Proposed Top-Level Structure

```json
{
  "app": {},
  "settings": {},
  "sessions": [],
  "groups": [],
  "known_hosts": {},
  "secrets": {},
  "metadata": {}
}
```

### 12.3 Example Config

```json
{
  "app": {
    "name": "DD-SSH",
    "config_version": 1
  },
  "settings": {
    "terminal": {
      "font_family": "monospace",
      "font_size": 11,
      "theme": "dark"
    },
    "behavior": {
      "open_sessions_in_tabs": true,
      "confirm_multi_exec": true
    },
    "sync": {
      "mode": "file",
      "config_path_user_selected": true
    }
  },
  "groups": [
    {
      "id": "dd-lab",
      "name": "DD-Lab"
    },
    {
      "id": "monitoring",
      "name": "Monitoring"
    }
  ],
  "sessions": [
    {
      "id": "nextcloud-backend",
      "name": "Nextcloud Backend",
      "group": "dd-lab",
      "host": "cloud.example.com",
      "port": 22,
      "username": "root",
      "auth": {
        "type": "key",
        "key_ref": "main-key"
      },
      "keepalive": {
        "enabled": true,
        "interval_seconds": 30
      },
      "terminal": {
        "font_family": "monospace",
        "font_size": 11
      }
    }
  ],
  "known_hosts": {
    "cloud.example.com:22": {
      "algorithm": "ssh-ed25519",
      "fingerprint": "SHA256:example",
      "first_seen": "2026-05-12T20:30:00Z",
      "last_seen": "2026-05-12T20:30:00Z"
    }
  },
  "secrets": {
    "mode": "plain-dev-only",
    "items": {
      "main-key": {
        "type": "private_key",
        "value": "-----BEGIN OPENSSH PRIVATE KEY-----\n...\n-----END OPENSSH PRIVATE KEY-----"
      }
    }
  },
  "metadata": {
    "created_by": "DD-SSH",
    "last_modified": "2026-05-12T20:30:00Z"
  }
}
```

### 12.4 Secrets Strategy

Initial development may use plain dev-only secrets for testing.

However, the config format should already be designed so that later encryption can be added without changing session structure.

Possible future encrypted form:

```json
{
  "secrets": {
    "mode": "encrypted",
    "kdf": "argon2id",
    "cipher": "aes-256-gcm",
    "salt": "base64-salt",
    "nonce": "base64-nonce",
    "data": "base64-encrypted-json"
  }
}
```

Important principle:

```text
Sessions reference secrets.
Secrets are stored separately inside the same JSON file.
```

This keeps the config clean and makes future encryption easier.

---

## 13. Config Locations

Default config paths should follow OS conventions:

```text
Linux:
~/.config/DD-SSH/dd-ssh.json

Windows:
%APPDATA%\DD-SSH\dd-ssh.json

macOS:
~/Library/Application Support/DD-SSH/dd-ssh.json
```

But DD-SSH should also support user-selected config file location.

This enables sync with:

```text
- Nextcloud sync folder
- WebDAV-mounted folder
- Syncthing
- Dropbox-like sync
- Git-managed private config repository
```

Recommended sync-friendly path example:

```text
~/Nextcloud/DD-SSH/dd-ssh.json
```

The app itself does not need built-in WebDAV in early versions. It only needs to read and write a normal file.

---

## 14. Sync Strategy

### 14.1 Initial Sync Model

DD-SSH should use file-based sync.

The application does not need to know whether the file is synced by Nextcloud, WebDAV, Syncthing, Dropbox, iCloud, or anything else.

The app only needs:

```text
- Open config file
- Save config file
- Detect external changes
- Reload when requested
- Create backup before save
```

### 14.2 Conflict Handling

Possible sync issue:

```text
Computer A edits dd-ssh.json.
Computer B edits dd-ssh.json.
Sync tool creates conflict.
```

Early version handling:

```text
- Always create timestamped backup before save
- Detect if file changed on disk since last load
- Ask user before overwriting
```

Future version handling:

```text
- Merge sessions by unique id
- Show conflict resolution dialog
- Preserve both conflicting versions
```

---

## 15. Known-Host Handling

Known-host verification is mandatory for a serious SSH client.

### 15.1 Unknown Host

When connecting to a new host:

```text
Unknown host key

Host: example.com:22
Fingerprint: SHA256:xxxxx

Do you trust this host?
[Trust once] [Trust permanently] [Cancel]
```

### 15.2 Changed Host Key

When a host key changes:

```text
WARNING: Host key changed

This may mean:
- Server was reinstalled
- DNS/IP changed
- Load balancer changed backend
- Possible man-in-the-middle attack

Default action: Cancel
```

The app must not silently accept changed host keys.

### 15.3 Storage

Known hosts can be stored inside `dd-ssh.json` for portability.

Future option:

```text
- Import from OpenSSH known_hosts
- Export to OpenSSH known_hosts
- Use system known_hosts optionally
```

---

## 16. Authentication Strategy

Initial supported methods:

```text
- Password authentication
- Private-key authentication
```

Later possible methods:

```text
- Keyboard-interactive authentication
- SSH agent support
- Hardware keys / FIDO2 if feasible
```

### 16.1 Password Auth

For early development:

```text
- Prompt user for password
- Optionally support dev-only plain password in config
```

For later versions:

```text
- Store encrypted password in JSON secrets block
- Or use OS keychain as optional backend
```

### 16.2 Private-Key Auth

Supported forms:

```text
- Key path per OS
- Embedded private key inside JSON secrets
```

Example key path config:

```json
"auth": {
  "type": "key",
  "key_path": {
    "linux": "~/.ssh/id_ed25519",
    "windows": "C:/Users/User/.ssh/id_ed25519",
    "macos": "~/.ssh/id_ed25519"
  }
}
```

Example key reference config:

```json
"auth": {
  "type": "key",
  "key_ref": "main-key"
}
```

---

## 17. Terminal Behavior

The terminal must behave like a real terminal, not like a text box.

It should support:

```text
- Interactive shell
- ANSI colors
- Cursor movement
- Fullscreen terminal apps
- UTF-8
- Resize
- Scrollback
- Copy/paste
- Ctrl+C for interrupt
- Ctrl+Shift+C for copy
- Ctrl+Shift+V for paste
```

Test commands for terminal correctness:

```bash
ls -la
uname -a
top
htop
nano test.txt
vim test.txt
mc
tmux
clear
watch -n 1 date
```

---

## 18. Tabs

Tabs should be simple and practical.

Each tab represents one active SSH session.

Each tab should show:

```text
- Session name
- Connection state
- Optional host/user indicator
```

Possible tab states:

```text
Disconnected
Connecting
Authenticating
Connected
Connection lost
Error
```

Each tab internally owns:

```text
- Terminal frontend
- SSH worker
- Session profile copy
- Current connection state
```

---

## 19. Keep-Alive

Keep-alive should be session-configurable.

Example config:

```json
"keepalive": {
  "enabled": true,
  "interval_seconds": 30
}
```

Behavior:

```text
- Send keep-alive while session is connected
- Stop keep-alive after disconnect
- Show connection lost if keep-alive fails or channel dies
```

Auto-reconnect should not be enabled in the first implementation unless explicitly designed and tested.

---

## 20. Multi-Exec Command

Multi-exec allows sending a command to multiple open tabs or selected sessions.

This is powerful and risky.

### 20.1 Required Behavior

The user should be able to:

```text
- Select multiple active tabs
- Type one command
- Preview targets
- Choose send mode
- Confirm
```

Send modes:

```text
1. Insert text only
2. Insert text + Enter
```

### 20.2 Safety Behavior

Dangerous command warning should trigger for commands containing patterns such as:

```text
rm
reboot
shutdown
mkfs
dd
systemctl restart
iptables
nft
firewall-cmd
passwd
userdel
```

The app should not block expert users permanently, but it should prevent accidental disasters.

### 20.3 Multi-Exec Log

Multi-exec actions should be logged locally:

```text
Timestamp
Command
Targets
Mode: insert only / insert + enter
```

No passwords or secrets should be written to logs.

---

## 21. UI Layout Concept

Initial layout:

```text
+------------------------------------------------------------+
| DD-SSH                                                     |
+------------------------------------------------------------+
| Toolbar: [Connect] [New Session] [Multi-Exec] [Settings]   |
+----------------------+-------------------------------------+
| Sessions             | Tabs                                |
|                      | [server1] [server2] [server3]       |
| DD-Lab               |                                     |
|  - Nextcloud         | Terminal area                       |
|  - Zabbix            |                                     |
| Lab                  |                                     |
|  - Test VM           |                                     |
+----------------------+-------------------------------------+
| Status: Connected to root@example.com                      |
+------------------------------------------------------------+
```

The UI should be boring in the best possible way.

Principles:

```text
- Clear over clever
- Practical over fancy
- Fast over animated
- Minimal but not ugly
```

---

## 22. Roadmap

### v0.0 — Planning

Goals:

```text
- Finalize blueprint
- Decide initial technology stack
- Define repository structure
- Define config format draft
- Define MVP boundaries
```

Deliverables:

```text
- README draft
- ARCHITECTURE.md
- ROADMAP.md
- CONFIG_FORMAT.md
- BUILDING.md
- AGENTS.md
```

### v0.1 — SSH Terminal Proof

Goals:

```text
- Qt window opens
- Manual connection dialog
- Connect to host with password
- Known-host basic prompt
- Open PTY/shell
- Display terminal output
- Send keyboard input
- Disconnect cleanly
```

Not included:

```text
- Session manager
- Tabs
- Private-key auth
- Multi-exec
- Packaging
```

Success criteria:

```text
User can connect to a Linux server and run:
ls -la
nano
htop
```

### v0.2 — Config and Session Manager

Goals:

```text
- Single dd-ssh.json config
- Load/save sessions
- Session sidebar
- Add/edit/delete session
- Manual import/export
```

Success criteria:

```text
User can create saved server entries and reconnect from the sidebar.
```

### v0.3 — Tabs and Private Keys

Goals:

```text
- Multiple terminal tabs
- Private-key authentication
- Key passphrase prompt
- Better session state handling
```

Success criteria:

```text
User can open multiple SSH tabs and authenticate using password or private key.
```

### v0.4 — Keep-Alive and Sync-Friendly Config

Goals:

```text
- Keep-alive support
- User-selected config path
- Detect external config changes
- Backup config before save
```

Success criteria:

```text
User can place dd-ssh.json in a Nextcloud/Syncthing folder and use it on multiple devices.
```

### v0.5 — Multi-Exec

Goals:

```text
- Select active tabs
- Send command to multiple tabs
- Preview targets
- Dangerous command warning
- Local multi-exec log
```

Success criteria:

```text
User can safely send the same command to multiple active SSH tabs.
```

### v0.6 — Cross-Platform Build Stabilization

Goals:

```text
- Linux build
- Windows build
- macOS build
- CMake cleanup
- Dependency documentation
```

Success criteria:

```text
Same source code builds and runs on Linux, Windows, and macOS.
```

### v0.7 — Packaging Preparation

Goals:

```text
- Linux AppImage experiment
- Linux .deb package experiment
- Windows installer experiment
- macOS .app/.dmg experiment
- Icons and app metadata
```

Success criteria:

```text
A non-developer can install and launch DD-SSH on at least one platform.
```

### v1.0 — First Public Release

Goals:

```text
- Stable core features
- Documentation
- Source release on GitHub
- Packaged release for primary OSes
- Basic release notes
- Known limitations clearly documented
```

Success criteria:

```text
DD-SSH can be used as a daily SSH client for normal server administration.
```

---

## 23. Distribution Plan

Target distribution channels:

```text
Source:
- GitHub repository
- GitHub Releases

Linux:
- AppImage
- .deb package
- Snap
- Homebrew/Linuxbrew if practical
- Flatpak later if useful

Windows:
- .exe installer
- portable .zip build
- Microsoft Store later if feasible

macOS:
- .app bundle
- .dmg installer
- Homebrew cask
- Apple notarized distribution
- Mac App Store later if feasible
```

Important distinction:

```text
Signed/notarized macOS DMG and Mac App Store release are not the same thing.
```

Store uploads should be treated as a later milestone. The application must first be stable outside the stores.

---

## 24. Signing and Certificates

Eventually needed:

```text
Windows:
- Code signing certificate
- Signed installer
- Possibly Microsoft Store account

macOS:
- Apple Developer account
- Code signing identity
- Notarization
- Hardened runtime considerations
- App Store review if targeting Mac App Store

Linux:
- Package signing if maintaining repository later
- Snap/Flatpak store accounts if used
```

This is important, but not an early coding blocker.

Project rule:

```text
Build first. Sign later. Store last.
```

---

## 25. Git and Branch Strategy

Recommended branches:

```text
main        stable code only
dev         active integration branch
feature/*   feature branches
fix/*       bugfix branches
release/*   release preparation
```

Every major feature should go through a pull request, even if the project is maintained by one person.

Why:

```text
- Cleaner history
- Easier code review
- Easier AI review later
- Easier rollback
- Better release discipline
```

---

## 26. AI Coding Agent Strategy

AI agents should be used as development helpers, not as part of DD-SSH v1.

### 26.1 Good Uses

```text
- Generate boilerplate C++/Qt code
- Review pull requests
- Explain compiler errors
- Write unit tests
- Improve CMake files
- Draft documentation
- Check cross-platform issues
- Suggest refactors
```

### 26.2 Bad Uses

```text
- Direct commits to main
- Handling real private keys or passwords
- Making architecture decisions without review
- Changing config format without documentation update
- Running destructive commands
```

### 26.3 AGENTS.md

The repository should include `AGENTS.md` with instructions for future AI coding agents.

Suggested content:

```text
Project: DD-SSH
Language: C++ / Qt 6 / CMake

Rules:
- Do not add Electron.
- Do not store secrets in logs.
- Do not hardcode platform-specific paths.
- Keep terminal frontend abstracted.
- Keep SSH logic outside UI classes.
- Every config format change must update docs/CONFIG_FORMAT.md.
- Every feature must preserve Linux/Windows/macOS portability.
- Prefer small focused classes.
- Do not modify main branch directly.
- Do not introduce telemetry.
```

---

## 27. Security Notes for Project Direction

Security is not the main implementation focus in the earliest proof-of-concept, but the architecture must not make future security impossible.

Important principles:

```text
- No secrets in logs
- No accidental Git commits of real config files
- No silent host key acceptance
- No automatic dangerous multi-exec
- No telemetry
- No hidden cloud sync
- No remote command execution without visible user action
```

Development convenience is acceptable in early prototypes, but unsafe behavior must be clearly marked as dev-only.

---

## 28. Testing Strategy

### 28.1 Manual Test Servers

Testing should include:

```text
- Local VM
- Local LAN Linux server
- Public VPS
- Server using password auth
- Server using private-key auth
- Server with changed host key scenario
```

### 28.2 Manual Terminal Tests

Commands:

```bash
ls -la
pwd
uname -a
clear
nano test.txt
vim test.txt
htop
top
mc
tmux
journalctl -f
watch -n 1 date
```

### 28.3 Automated Tests

Automated tests should focus on logic that does not require a real terminal GUI:

```text
- ConfigManager load/save
- Config version validation
- SessionManager add/edit/delete
- KnownHostsManager compare fingerprints
- MultiExecManager command classification
- Path handling across OSes
```

---

## 29. Build Strategy

### 29.1 Local Development Order

Recommended development order:

```text
1. Linux first for fast iteration
2. Windows early reality check
3. macOS build as soon as core works
```

Linux is the workshop, not the final-only target.

The source must stay cross-platform from day one.

### 29.2 CI/CD Direction

Future GitHub Actions matrix:

```text
- ubuntu-latest
- windows-latest
- macos-latest
```

Initial CI target:

```text
- Configure CMake
- Build project
- Run unit tests
```

Later CI target:

```text
- Build packages
- Upload artifacts
- Create release drafts
```

---

## 30. Documentation Plan

Required documents:

```text
README.md
- What DD-SSH is
- Main features
- Screenshots later
- Basic build/install

ARCHITECTURE.md
- Layers
- Components
- Data flow
- Threading model

ROADMAP.md
- Version plan
- Current milestone
- Future ideas

CONFIG_FORMAT.md
- dd-ssh.json schema
- Examples
- Versioning rules

BUILDING.md
- Linux build
- Windows build
- macOS build

PACKAGING.md
- AppImage
- deb
- Snap
- Windows installer
- macOS DMG

SECURITY_NOTES.md
- Known-host handling
- Secrets strategy
- Multi-exec safety

RELEASE_PROCESS.md
- Version bump
- Changelog
- Build artifacts
- Signing checklist
```

---

## 31. Naming and Branding Notes

Working name:

```text
DD-SSH
```

Possible tagline:

```text
A clean cross-platform SSH client for people who just want to connect.
```

Alternative tagline:

```text
SSH without the circus.
```

Tone:

```text
Practical
Clean
Slightly witty
Professional enough for stores
Not childish
Not enterprise-bloated
```

Avoid using PortX branding publicly. DD-SSH can be inspired by the workflow idea, but it must have its own identity, code, name, design, and documentation.

---

## 32. Initial Milestone Checklist

Before coding:

```text
[ ] Confirm project name
[ ] Confirm initial stack
[ ] Create GitHub repository
[ ] Add README.md
[ ] Add LICENSE
[ ] Add this blueprint as docs/PROJECT_BLUEPRINT.md
[ ] Add AGENTS.md
[ ] Add initial CMakeLists.txt
[ ] Add empty Qt app skeleton
```

Before v0.1 is considered successful:

```text
[ ] App launches
[ ] Manual connect dialog exists
[ ] libssh connects to server
[ ] Known-host prompt appears for unknown host
[ ] Password auth works
[ ] PTY shell opens
[ ] Terminal frontend displays output
[ ] Keyboard input reaches remote shell
[ ] Terminal resize works
[ ] Disconnect works
```

---

## 33. Golden Rules

```text
1. Do not build everything at once.
2. Do not optimize before the first terminal works.
3. Do not add SFTP in v1.
4. Do not add split screen in v1.
5. Do not add AI features inside the app in v1.
6. Do not let UI classes contain SSH protocol logic.
7. Do not let SSH worker block the GUI thread.
8. Do not silently accept changed host keys.
9. Do not write secrets to logs.
10. Do not break cross-platform portability for short-term convenience.
11. Do not change config format without updating documentation.
12. Do not chase stores before the app is stable.
```

---

## 34. Definition of Done for DD-SSH v1.0

DD-SSH v1.0 is done when:

```text
- Linux build works
- Windows build works
- macOS build works
- User can manage saved sessions
- User can connect using password
- User can connect using private key
- Known-host verification works
- Tabs work
- Copy/paste works
- Keep-alive works
- Config import/export works
- Config can be synced as one JSON file
- Multi-exec works with confirmation and preview
- Documentation exists
- Basic packages/installers exist
- Known limitations are documented
```

DD-SSH v1.0 does not need:

```text
- SFTP
- Split screen
- Built-in cloud sync
- Mac App Store approval
- Microsoft Store approval
- Mobile apps
- Plugin system
```

---

## 35. Final Project Direction

DD-SSH should be developed as a serious open-source cross-platform product, but with a disciplined MVP mindset.

The long-term dream can include stores, signing, polished installers, Homebrew, Snap, deb packages, macOS DMG, Windows installer, and maybe store listings.

But the short-term mission is much simpler:

```text
Make one SSH terminal tab work correctly.
Then make it pleasant.
Then make it portable.
Then make it distributable.
```

This blueprint is the foundation. Future decisions should be compared against it. If a new idea helps the core mission, it can be added. If it bloats the project before the core works, it waits.

DD-SSH should become:

```text
A clean, practical, cross-platform SSH client with session management,
one-file JSON portability, safe multi-exec, and no unnecessary nonsense.
```

