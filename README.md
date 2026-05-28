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

**Development checkpoint:** `dev 0.1.6.6`
**Codename:** Andromeda
**Milestone:** MF 0.2 candidate — Real Terminal Foundation
**Current phase:** macOS DMG/dependency polish

DD-SSH is now in its early packaging phase. It is not a stable 1.0 release yet, but the core workflow is functional and has been validated on Linux, Windows 10, Windows 11, and an Intel macOS build machine. Native Windows Debug/Release builds, a copied standalone Windows deployment folder, a first Debian package, and a first macOS Intel `.app` / `.dmg` deployment experiment have been tested.

The current macOS polish checkpoint keeps the 0.1.6.3 SSH trust-chain hardening and the 0.1.6.4 repo-hygiene rules intact, then improves the macOS tester package path:

- `dev 0.1.5.6` proved the Windows standalone deployment folder can run outside the build tree without manually extending `PATH`.
- `dev 0.1.5.7` fixed portable `known_hosts` behavior so one shared `dd-ssh.json` can carry multiple legitimate host-key algorithms per `host:port`.
- `dev 0.1.5.8` fixed a Windows/libssh handshake failure against newer OpenSSH servers that advertise ML-KEM/SNTRUP key-exchange algorithms before classic curve25519/ecdh algorithms.
- `dev 0.1.6.1` and `dev 0.1.6.1.1` added the first Debian package workflow, screenshots, and packaging tutorial.
- `dev 0.1.6.2` added the first macOS Intel app/DMG build and deployment documentation, including a DMG layout with an Applications shortcut.
- `dev 0.1.6.3` verifies the approved SSH host key again in the real authentication/shell connection before any password or private key is sent.
- `dev 0.1.6.4` adds `.gitignore` protection, release-artifact documentation, and checksum helpers for Linux, macOS, and Windows.
- `dev 0.1.6.6` adds optional diagnostic logging for tester/debug workflows: logging is OFF by default, can be enabled from Settings, writes to a standard per-user log folder, and Help → Open Log Folder opens the logs.

In `dev 0.1.6.6`, runtime/SSH behavior is intentionally unchanged from `dev 0.1.6.3`. The focus is an optional diagnostic logging foundation for real tester debugging without storing secrets, terminal input, or terminal output in logs.

The portable known-host model now supports multi-key storage per `host:port`:

- old single-key known-host entries are migrated when saved
- a new legitimate key algorithm is offered as **Trust additional key** instead of forcing **Replace stored key**
- true same-algorithm fingerprint mismatches still show the strong SSH host-key-changed warning
- `Trust once` continues for the current attempt without saving the new key

On Windows builds, DD-SSH applies a conservative libssh KEX override before `ssh_connect()` so affected Windows/vcpkg/libssh builds can connect to modern OpenSSH servers where the default algorithm proposal previously failed with `Failed to construct client init buffer`. The server-side workaround used during debugging is no longer required for the validated regression case.

The previous polish pass also added two user-safety clarifications:

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




## Screenshots

These screenshots show DD-SSH installed and running from the first Debian package experiment on Linux.

### Welcome screen and saved sessions

![DD-SSH welcome screen with saved session sidebar](docs/screenshots/dd-ssh-welcome-overview.png)

The Welcome tab summarizes the current Andromeda milestone, available workflows, documentation entry points, and the saved-session sidebar. This view is useful as a quick project/status dashboard after installing the package.

### Connected xterm.js SSH terminal

![DD-SSH connected xterm.js SSH terminal](docs/screenshots/dd-ssh-terminal-connected.png)

A saved session is opened as a real xterm.js terminal backed by a libssh shell channel. The terminal shows the connection state, target, local xterm.js renderer, PTY resize state, SSH authentication progress, and a live Ubuntu shell.

### Edit saved session dialog

![DD-SSH edit saved session dialog](docs/screenshots/dd-ssh-edit-session-dialog.png)

Saved sessions can be edited without retyping existing plaintext secrets. Leaving the password/private-key fields empty keeps the saved secret; entering a new password or key replaces it in `dd-ssh.json`.

### Settings dialog

![DD-SSH settings dialog](docs/screenshots/dd-ssh-settings-dialog.png)

The Settings dialog exposes the active config path, app theme selection, terminal font settings, quick toolbar visibility, and rotating config-backup options. The config folder can be opened directly from here.

### Dark theme terminal

![DD-SSH dark theme terminal](docs/screenshots/dd-ssh-dark-theme-terminal.png)

DD-SSH can follow a dark Qt app theme while keeping the xterm.js terminal in its dark terminal style. This view shows the normal connected-session layout after theme changes are applied.

### About dialog

![DD-SSH About dialog](docs/screenshots/dd-ssh-about-dialog.png)

The About dialog reports the current development version, codename, milestone, linked libssh backend version, and the exact config file path. It is the quickest sanity check after installing a new `.deb` package.

See [Screenshots](docs/SCREENSHOTS.md) for the same gallery with longer descriptions.

---

## Public alpha preparation

The current development line is preparing for a public alpha tag:

```text
v0.2.0-alpha — Andromeda
MF 0.2 — Real Terminal Foundation
```

Before tagging, run:

- [Linux Packaging Guide](docs/LINUX_PACKAGING.md)
- [Debian Package Tutorial](docs/DEBIAN_PACKAGE_TUTORIAL.md)
- [macOS Build Guide](docs/MACOS_BUILD.md)
- [macOS Deployment Guide](docs/MACOS_DEPLOYMENT.md)
- [Screenshots](docs/SCREENSHOTS.md)
- [Public Alpha Checklist](docs/PUBLIC_ALPHA_CHECKLIST.md)
- [Test Matrix](docs/TEST_MATRIX.md)
- [Windows Build Guide](docs/WINDOWS_BUILD.md)
- [Windows Deployment Guide](docs/WINDOWS_DEPLOYMENT.md)
- [Release Artifacts](docs/RELEASE_ARTIFACTS.md)
- [macOS Build Guide](docs/MACOS_BUILD.md)
- [macOS Deployment Guide](docs/MACOS_DEPLOYMENT.md)
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
- macOS `.icns` / `.app` bundle / `.dmg` deployment prep for Intel builds
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
- macOS Intel build and first `.dmg` validation pass
- Final signed installers / official repositories
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
- [Release Artifacts](docs/RELEASE_ARTIFACTS.md)
- [macOS Build Guide](docs/MACOS_BUILD.md)
- [macOS Deployment Guide](docs/MACOS_DEPLOYMENT.md)
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



## Debian package quick path

The first local `.deb` packaging workflow is documented in detail in [Debian Package Tutorial](docs/DEBIAN_PACKAGE_TUTORIAL.md).

Short version:

```bash
./scripts/linux-build-release.sh
./scripts/linux-package-deb.sh
sudo apt install ./dist/deb/dd-ssh_0.1.6.5_amd64.deb
dd-ssh
```

The package installs the app, desktop launcher, icons, README, license, Markdown documentation, and screenshots. It does not package your personal `dd-ssh.json` config.

---

## macOS Intel app/DMG quick path

The first macOS packaging workflow is documented in [macOS Build Guide](docs/MACOS_BUILD.md) and [macOS Deployment Guide](docs/MACOS_DEPLOYMENT.md).

Short version on the validated Intel build machine:

```bash
./scripts/macos-build-release.sh
./scripts/macos-deploy-release.sh
open dist/macos/DD-SSH.app
open dist/macos/DD-SSH-0.1.6.5-macOS-x86_64.dmg
```

The generated DMG contains `DD-SSH.app` and an `Applications` shortcut so testers can drag the app into `/Applications`. The first macOS package is unsigned and not notarized; Gatekeeper may require right-click → Open on tester machines. The initial target is Intel macOS; Apple Silicon support is expected via Rosetta for this build, with native arm64/universal builds planned later.

---

