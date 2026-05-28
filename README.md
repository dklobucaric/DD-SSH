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

**Development checkpoint:** `dev 0.1.8.1.2`
**Codename:** Andromeda
**Milestone:** Tester release polish
**Current phase:** Third-party tester preparation

DD-SSH is now moving from the closed terminal-foundation baseline into the File Transfer / File Manager development track. It is not a stable 1.0 release yet, but the core terminal workflow is functional and has been validated on Linux, Windows 10/11, and Intel macOS. Native Windows builds, a standalone Windows deployment folder, a Debian package, and a macOS Intel `.app` / `.dmg` tester flow have been tested.

The current checkpoint is a tester-release polish step: `dev 0.1.8.1.2` keeps the accepted `dev 0.1.8.1.1` File Manager baseline and prepares it for third-party alpha testing. It updates tester-facing documentation, build/test notes, known limitations, and in-app About/Welcome text. No transfer engine, terminal runtime, queue behavior, known-host handling, Windows KEX workaround, upload/download logic, or folder scan logic is changed. Folder transfer remains experimental and should be validated first with small test folders.

Recent checkpoints:

- `dev 0.1.8.1.2` is a tester-release polish checkpoint. It prepares the accepted file manager and folder/queue baseline for third-party testing with clearer docs, tester checklist, known limitations, and alpha safety notes. No runtime transfer behavior is intentionally changed.
- `dev 0.1.8.1.1` consolidates File Manager queue actions into two clearer buttons: local `Queue upload` and remote `Queue download`. Selected files are queued directly; selected folders are confirmed and recursively expanded into normal queue items.
- `dev 0.1.8.1` adds the first recursive folder-transfer experiment by expanding selected folders into normal queue items. Destination folders are created as queue items and symlinks are skipped.
- `dev 0.1.8.0.3` adds `Retry selected` for finished queue items so selected `Done`, `Failed`, `Cancelled`, or `Skipped` transfers can be moved back to `Pending` and run again.
- `dev 0.1.8.0.2` adds queue overwrite/skip-all decisions so repeated overwrite prompts do not have to be answered one file at a time.
- `dev 0.1.8.0.1` fixes queue overwrite prompt ordering so overwrite questions appear in front of the user instead of being hidden by the queue progress dialog.
- `dev 0.1.8.0` adds the first SFTP transfer queue foundation: queue multiple individual downloads/uploads, run them sequentially, track status, remove selected, and clear finished items.
- `dev 0.1.7.8` polishes single-file SFTP transfer progress and cancel feedback: transfer dialogs show progress, speed, elapsed time, completion summaries, and clearer cancellation messages.
- `dev 0.1.5.6` proved the Windows standalone deployment folder can run outside the build tree without manually extending `PATH`.
- `dev 0.1.5.7` fixed portable `known_hosts` behavior so one shared `dd-ssh.json` can carry multiple legitimate host-key algorithms per `host:port`.
- `dev 0.1.5.8` fixed a Windows/libssh handshake failure against newer OpenSSH servers that advertise ML-KEM/SNTRUP key-exchange algorithms before classic curve25519/ecdh algorithms.
- `dev 0.1.6.3` verifies the approved SSH host key again in the real authentication/shell connection before any password or private key is sent.
- `dev 0.1.6.4` adds `.gitignore` protection, release-artifact documentation, and checksum helpers for Linux, macOS, and Windows.
- `dev 0.1.6.5` improves the macOS DMG/dependency flow and validates the simple DMG tester install path.
- `dev 0.1.6.6` adds optional diagnostic logging for tester/debug workflows. Logging is OFF by default and can be enabled from Settings.
- `dev 0.1.6.7` adds a compact Session Traffic indicator in the status bar for the active terminal tab, showing live received/sent rates and totals.
- `dev 0.1.6.8` adds config import/export safety previews while keeping the human-readable JSON format unchanged.
- `dev 0.1.6.9` is a small bugfix stabilization checkpoint for toolbar and config-dialog button labels.
- `dev 0.1.7.7` adds the first single-file SFTP upload path: select one local file and upload it into the current remote folder with overwrite warning and basic progress.
- `dev 0.1.7.6.1` polishes single-file download: remote Size sorting now uses raw bytes, and the completion dialog shows formatted size plus raw byte count.
- `dev 0.1.7.6` adds the first single-file SFTP download path: select one remote file and download it into the current local folder with overwrite warning and basic progress.
- `dev 0.1.7.5` added the first two-panel read-only file manager foundation: local browser on the left, remote SFTP browser on the right, both read-only and no transfers yet.
- `dev 0.1.7.4.1` polishes the first read-only SFTP browser: exit safety now lists SFTP browser tabs, alternating table rows are disabled for dark-theme readability, and tab scroll-button hints are forced for crowded tab bars on macOS/Windows/Linux.
- `dev 0.1.7.4` added the first graphical read-only SFTP remote browser: saved session → known-host preflight → host-key verification before auth → SFTP listing table with path, Go, Up, Refresh, and double-click folder navigation.
- `dev 0.1.7.3` adds the first SFTP connection proof of concept: saved session → known-host preflight → host-key verification before auth → SFTP init → remote `.` listing in a text tab.
- `dev 0.1.7.2` starts the File Transfer / File Manager track with architecture docs and a safe File Manager placeholder; it does not open SFTP yet.
- `dev 0.1.7.1` hardens native xterm.js paste handling: toolbar Paste, right-click paste, Ctrl+Shift+V, and macOS Command+V now route through the same DD-SSH safe paste path and should not leak bracketed paste markers such as `^[[200~` into the remote shell.
- `dev 0.1.7.0` hardens terminal transport: SSH output is carried as bytes to the WebEngine terminal, decoded with a streaming UTF-8 decoder, and input writes are partial-write aware so large paste/input is not silently truncated.

In `dev 0.1.8.1.2`, DD-SSH keeps the accepted `dev 0.1.8.1.1` queue/file-manager behavior and focuses on tester readiness. Queue processing remains sequential and conservative while the File Manager controls use local `Queue upload` and remote `Queue download` around the folder-transfer experiment. Folder transfer is still implemented as scan → expand into queue items → transfer one item at a time. No parallel execution, resume, sync engine, permission/timestamp preservation, symlink following, or SFTP Session Traffic integration yet.

The future file-transfer design is documented in `docs/FILE_TRANSFER_ARCHITECTURE.md`. Third-party tester flow is documented in `docs/TESTER_CHECKLIST_0.1.8.1.2.md`. File transfer will use libssh SFTP APIs, not shell command parsing hacks. Terminal tabs and future File Manager tabs are intentionally separated so the working terminal foundation remains stable.

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

If one or more SSH terminal tabs or SFTP file manager tabs are open, closing the app with the window close button or `File → Exit` asks for confirmation before closing them.

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
- SFTP upload/download and two-panel local/remote file transfer runtime
- Split panes
- Multi-Exec command sending
- Keep-alive settings
- Portable mode next to binary
- Custom config path picker
- Apple Developer ID signing/notarization
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

DD-SSH should stay focused. Early versions intentionally avoid split panes, cloud lock-in, telemetry, complex sync engines, and bloated enterprise dashboards. File transfer is now being added carefully through libssh SFTP, one small checkpoint at a time.



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

