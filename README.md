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

**Development checkpoint:** `dev 0.1.8.7`  
**Codename:** Andromeda  
**Milestone:** Release/tester packaging polish  
**Current phase:** Release/tester packaging polish for File Manager alpha

DD-SSH is currently in the Andromeda tester-build line: the terminal foundation is stable, and the SFTP File Manager alpha is ready for wider Linux/Windows/macOS tester validation. The app has been validated across Linux, Windows 10/11, and Intel macOS through earlier checkpoints. Current packaging targets are a Linux `.deb`, a Windows portable deployment folder/ZIP, and an unsigned Intel macOS `.app` / `.dmg` tester flow.

The current checkpoint, `dev 0.1.8.7`, does not add risky runtime behavior. It refreshes the README, Welcome screen, feature inventory, test matrix, release/tester packaging checklist, and tester checklist so Linux/Windows/macOS builds can be packaged and handed to testers cleanly. It keeps the accepted `dev 0.1.8.6.4` queue/delete ordering behavior and the `dev 0.1.8.6.2` multiline paste fix.

### Current feature inventory

Core SSH/session features:

- saved sessions in one portable human-readable `dd-ssh.json`
- portable plaintext `plain-v1` secrets for early alpha portability
- password and private-key authentication
- SSH known-host verification before authentication
- multi-key known-host portability per `host:port`
- saved session create/edit/delete workflows
- duplicate saved-session warning
- config import/export/restore and corrupt-config recovery
- optional diagnostic logging, OFF by default
- Open Config Folder and Open Log Folder helpers

Terminal features:

- xterm.js terminal renderer with bundled local assets
- SSH PTY resize sync
- terminal lifecycle/disconnect handling
- tested full-screen terminal apps such as `htop`, `nano`, `vim`, `top`, and `clear`
- native paste routing for toolbar Paste, right-click paste, Ctrl+Shift+V, and macOS Command+V
- multiline paste newline handling for `nano`, YAML, and config editing
- Ctrl+C remote interrupt behavior
- active terminal Session Traffic status-bar widget

File Manager alpha features:

- two-panel local/remote File Manager
- remote browsing through libssh SFTP, not shell command parsing
- queue-first upload and download workflow
- recursive folder upload/download experiment via scan → queue items
- sequential one-item-at-a-time transfer queue
- Retry selected for Done/Failed/Cancelled/Skipped items
- Overwrite / Skip / Overwrite all / Skip all
- overwrite metadata dialogs with existing/incoming size and modified time
- local and remote queued delete for regular files, symlinks, and empty directories
- destructive delete confirmations shown only when the queue reaches the delete item
- diagnostic SFTP/file-transfer logs when logging is enabled

Packaging/tester features:

- Linux release build and Debian package helper
- macOS Intel `.app`/`.dmg` helper with otool dependency audit report
- Windows portable deployment helper
- Windows portable ZIP helper for tester artifacts
- checksum helpers for Linux/macOS/Windows release assets
- tester checklist and known limitations documentation

### Important alpha limits

The File Manager is useful but still intentionally conservative. It does **not** implement sync/mirror, partial-transfer resume, parallel transfers, recursive non-empty folder delete, rename, chmod/chown, permission/timestamp preservation, symlink following in folder transfer, or SFTP traffic integration in the Session Traffic widget yet.

### Recent checkpoints

- `dev 0.1.8.7` refreshes documentation, Welcome/About status text, tester feature inventory, and packaging/tester checklists for Linux/Windows/macOS handoff.
- `dev 0.1.8.6.4` fixes queue delete confirmation ordering so upload/download overwrite prompts and delete confirmations appear in the same order as queue items.
- `dev 0.1.8.6.3` polishes UI wording: saved-session context menu labels are now `Open terminal`, `Open file manager`, and `Open fallback shell`; the About dialog adds `Developer: DD-LAB vl. Dalibor Klobučarić`.
- `dev 0.1.8.6.2` fixes multiline terminal paste newline handling for full-screen editors such as nano.
- `dev 0.1.8.6.1` fixes macOS/Linux artifact version detection so package filenames follow `DD_SSH_VERSION_STRING`.
- `dev 0.1.8.6` adds conservative local delete queue support and simplifies File Manager delete labels.
- `dev 0.1.8.5` adds remote delete queue support for regular files, symlinks, and empty directories with destructive confirmation and diagnostic logging.
- `dev 0.1.8.4` adds overwrite metadata dialogs.
- `dev 0.1.8.3.1` polishes SFTP upload logging around overwrite preflight checks.
- `dev 0.1.8.3` adds explicit SFTP/file-transfer diagnostic logging while keeping logs free of secrets and file contents.
- `dev 0.1.8.2` adds file manager safety polish for tester runs.
- `dev 0.1.8.1.1` consolidates File Manager queue actions into local `Queue upload` and remote `Queue download`.
- `dev 0.1.8.1` adds the first recursive folder-transfer experiment by expanding folders into queue items.
- `dev 0.1.8.0.x` builds the transfer queue foundation, Overwrite all / Skip all, Retry selected, and queue stabilization.
- `dev 0.1.7.x` builds the SFTP probe, remote browser, two-panel File Manager, single-file download/upload, and progress/cancel polish.
- `dev 0.1.7.1` remains the closed terminal foundation baseline after native paste hardening.

The future file-transfer design is documented in `docs/FILE_TRANSFER_ARCHITECTURE.md`. Third-party tester flow is documented in `docs/TESTER_CHECKLIST_0.1.8.7.md`. File transfer uses libssh SFTP APIs, not shell command parsing hacks. Terminal tabs and File Manager tabs are intentionally separated so the working terminal foundation remains stable.

### Important session-save behavior

DD-SSH saves a new session only after a successful SSH authentication test.

If the host is unreachable, the port is wrong, the username/password is wrong, the private key is invalid, or the known-host check does not allow continuing, the session is not written to `dd-ssh.json`.

### Exit safety

If SSH terminal tabs, SFTP file manager tabs, or pending/running queue work exist, closing the app with the window close button or `File → Exit` asks for confirmation.

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

