# DD-SSH

**DD-SSH** is a clean cross-platform SSH client and session manager built with Qt/C++/CMake/libssh.

The goal is simple:

```text
Open the app.
Double-click a saved server.
Get a real SSH terminal.
Work.
```

DD-SSH is designed for practical sysadmin use: saved sessions, one portable JSON config, known-host checking, password/private-key authentication, xterm.js terminal tabs, and safe config handling.

---

## Current status

**Development checkpoint:** `dev 0.1.5.7`
**Codename:** Andromeda
**Milestone:** MF 0.2 candidate — Real Terminal Foundation
**Current phase:** Known-host multi-key portability polish

DD-SSH is now close to a **public alpha**. It is not a stable 1.0 release yet, but the core workflow is functional and tested on Linux. Native Windows Debug and Release builds have also been validated with MSVC, Qt 6.11.1, Qt WebEngine/WebChannel/Positioning, vcpkg/libssh, and pkgconf.

The current bugfix pass focuses on portable `known_hosts` behavior. A single shared `dd-ssh.json` must work across Linux, Windows 10, Windows 11, and later macOS even when libssh negotiates a different legitimate server host-key algorithm on each platform.

The 0.1.5.7 known-host fix adds multi-key storage per `host:port`:

- old single-key known-host entries are migrated when saved
- a new legitimate key algorithm is offered as **Trust additional key** instead of forcing **Replace stored key**
- true same-algorithm fingerprint mismatches still show the strong SSH host-key-changed warning
- `Trust once` continues for the current attempt without saving the new key

The previous polish pass added two user-safety clarifications:

- new saved sessions are stored only after a successful SSH authentication test
- closing DD-SSH with active SSH terminal tabs asks for confirmation before disconnecting them

On Windows, the first xterm.js terminal tab may take a few seconds while Qt WebEngine initializes; DD-SSH reports that startup state more clearly:

```text
saved session → plaintext secret from dd-ssh.json → known_hosts check → SSH auth → xterm.js terminal → PTY resize → real shell
```

### Important session-save behavior

DD-SSH saves a new session only after a successful SSH authentication test.

If the host is unreachable, the port is wrong, the username/password is wrong, the private key is invalid, or the known-host check does not allow continuing, the session is not written to `dd-ssh.json`.

This prevents broken or unverified sessions from being saved as normal connection profiles.

### Exit safety

If one or more SSH terminal tabs are still connected, closing the app with the window close button or `File → Exit` asks for confirmation before disconnecting them.

---



## Public alpha preparation

The current development line is preparing for a public alpha tag:

```text
v0.2.0-alpha — Andromeda
MF 0.2 — Real Terminal Foundation
```

Before tagging, run:

- [Public Alpha Checklist](docs/PUBLIC_ALPHA_CHECKLIST.md)
- [Test Matrix](docs/TEST_MATRIX.md)
- [Windows Build Guide](docs/WINDOWS_BUILD.md)
- [Windows Deployment Guide](docs/WINDOWS_DEPLOYMENT.md)
- [Release Notes Draft](docs/RELEASE_NOTES_v0.2.0-alpha.md)

GitHub issue templates are included for bug reports, terminal issues, config/recovery issues, and feature requests.

---

## Big security warning

Early DD-SSH builds intentionally support **portable plaintext secrets**.

That means saved passwords and private keys may be stored inside:

```text
dd-ssh.json
```

under:

```json
"secrets": {
  "mode": "plain-v1",
  "items": {}
}
```

This is convenient for portability, but it is **not secure for untrusted machines**.

Do not commit your real `dd-ssh.json` to Git.
Do not share it publicly.
Use this mode only on trusted computers.

Future versions may add encrypted/master-password storage while preserving the current session reference structure.

See [Security Notes](docs/SECURITY_NOTES.md).

---

## What works now

### Sessions and config

- Saved sessions loaded from `dd-ssh.json`
- One portable JSON config containing sessions, known_hosts, settings, metadata, and plaintext secrets
- Password authentication
- Private-key authentication
- Known-host trust handling
- Create/edit/delete saved sessions
- Duplicate `username@host:port` warning when saving sessions
- Double-click saved session opens xterm.js terminal by default
- Manual auth test remains available from the Session/context menu

### Terminal

- Local bundled xterm.js renderer through Qt WebEngine
- No CDN dependency for terminal assets
- FitAddon + SSH PTY resize sync
- Password and private-key terminal sessions
- Reconnect after disconnect/reboot
- Terminal lifecycle cleanup
- Tested terminal apps:
  - `htop`
  - `nano`
  - `vim`
  - `top`
  - `clear`
  - `stty size`

### Settings and UI

- Cross-platform app icon integration
- Qt window icon resource
- Windows `.ico` / `.rc` executable icon prep
- Linux PNG icon resources for future `.desktop` packaging
- macOS `.icns` and `.iconset` prep for future app bundles
- App theme: System / Light / Dark
- Terminal font family and size for newly opened terminal tabs
- Optional quick action toolbar
- Settings saved to `dd-ssh.json`
- File menu config actions:
  - Open Config Folder
  - Export Config
  - Import Config
  - Restore Latest Backup
  - Exit

### Windows validation

- Native Windows build confirmed using MSVC x64 and Ninja
- Qt 6.11.1 MSVC 2022 64-bit tested with WebEngine, WebChannel, and Positioning
- `libssh` provided by vcpkg
- `pkgconf` used for current CMake/libssh discovery
- Windows config path uses AppData/Local style location via Qt
- xterm.js terminal, SSH connection, and `htop` were confirmed working on Windows
- First xterm/WebEngine terminal startup may be slower than Linux; later terminals are faster
- RAM usage is higher on Windows because Qt WebEngine embeds a Chromium-based engine

See [Windows Build Guide](docs/WINDOWS_BUILD.md).

### Config safety

- Linux default config path:

```text
~/.config/DD-LAB/DD-SSH/dd-ssh.json
```

- Rotating backups before config saves
- Import creates pre-import backup
- Restore latest valid backup
- Corrupt config recovery dialog
- Corrupt config is never overwritten automatically
- Create fresh config option moves corrupt config aside first

---

## What does not exist yet

DD-SSH is intentionally still limited. Not implemented yet:

- Encrypted secrets / master password
- SSH agent support
- Keyboard-interactive auth polish
- SFTP
- Split panes
- Multi-Exec command sending
- Keep-alive settings
- Portable mode next to binary
- Custom config path picker
- macOS validation pass
- Packaging/installers
- Signed releases
- Full public release process

---

## Quick start for testers

### 1. Build

On Debian/Ubuntu/Linux Mint style systems:

```bash
sudo apt update
sudo apt install -y \
  build-essential \
  cmake \
  ninja-build \
  git \
  pkg-config \
  qt6-base-dev \
  qt6-base-dev-tools \
  qt6-tools-dev \
  qt6-tools-dev-tools \
  qt6-webengine-dev \
  libssh-dev

cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

See [Building](docs/BUILDING.md) and [Windows Build Guide](docs/WINDOWS_BUILD.md).

### 2. Create a session

Use:

```text
Session → New Session
```

Enter host, port, username, and auth method. After successful auth, DD-SSH saves the session to `dd-ssh.json`.

### 3. Open terminal

Double-click a saved session in the sidebar.

Expected result: an xterm.js terminal opens and connects to the saved SSH target.

### 4. Test terminal

```bash
whoami
hostname
stty size
htop
nano /tmp/dd-ssh-test.txt
vim /tmp/dd-ssh-test.txt
clear
exit
```

### 5. Backup/export config

Use:

```text
File → Export Config...
```

Exported config may contain plaintext passwords and private keys.

---

## Main use cases

### Use case: everyday SSH access

Save frequently used SSH targets once, then double-click from the sidebar to open a terminal tab.

### Use case: portable personal SSH profile

Copy `dd-ssh.json` to another trusted machine and keep sessions, known_hosts, settings, passwords, and private keys together.

### Use case: quick terminal health check

Open a session and run:

```bash
uptime
df -h
free -m
systemctl status nginx --no-pager
```

### Use case: recovery after broken config

If `dd-ssh.json` becomes invalid, DD-SSH warns, refuses to overwrite it, lists backups, and lets the user restore the latest valid backup or create a fresh config.

### Use case: public alpha testing

Run the test matrix in [Test Matrix](docs/TEST_MATRIX.md), report failures, and include OS/build details.

More examples: [Use Cases](docs/USE_CASES.md).

---

## Documentation map

Start here:

- [Getting Started](docs/GETTING_STARTED.md)
- [User Guide](docs/USER_GUIDE.md)
- [Use Cases](docs/USE_CASES.md)
- [Features and Limitations](docs/FEATURES.md)
- [Known Limitations](docs/KNOWN_LIMITATIONS.md)
- [Config Management](docs/CONFIG_MANAGEMENT.md)
- [Config Format](docs/CONFIG_FORMAT.md)
- [Security Notes](docs/SECURITY_NOTES.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Building](docs/BUILDING.md)
- [Windows Build Guide](docs/WINDOWS_BUILD.md)
- [Windows Deployment Guide](docs/WINDOWS_DEPLOYMENT.md)
- [Roadmap](docs/ROADMAP.md)
- [Test Matrix](docs/TEST_MATRIX.md)
- [Public Alpha Checklist](docs/PUBLIC_ALPHA_CHECKLIST.md)
- [Release Notes Draft](docs/RELEASE_NOTES_v0.2.0-alpha.md)
- [Changelog](docs/CHANGELOG.md)

---

## Codename roadmap

```text
0.0.x — Launchpad / early prototype history
0.1.x — Andromeda / current MF 0.2 candidate line
0.2.x — Orion
0.3.x — Vega
0.4.x — Cassiopeia
1.0.x — Apollo
```

Current line: **0.1.x Andromeda**.

---

## Project philosophy

```text
Fast to open.
Fast to connect.
Easy to understand.
Easy to back up.
Portable when needed.
Hard to accidentally destroy things.
```

DD-SSH should stay focused. Early versions intentionally avoid SFTP, split panes, cloud lock-in, telemetry, and bloated enterprise dashboards.
