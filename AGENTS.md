# AGENTS.md

Guidelines for AI coding agents working on DD-SSH.

## Project

DD-SSH is a cross-platform SSH client and session manager.

Current checkpoint:

```text
Version: dev 0.1.5.2
Codename: Andromeda
Milestone: MF 0.2 candidate
```

## Stack

- C++20
- Qt 6
- CMake
- libssh
- Qt WebEngine + Qt WebChannel
- local bundled xterm.js assets
- JSON config

## Hard rules

- Do not add Electron.
- Do not add telemetry.
- Do not store secrets in logs.
- Do not print password/private-key values.
- Do not hardcode Linux-only paths without abstraction.
- Use Qt standard paths for config locations.
- Do not silently accept changed host keys.
- Do not overwrite corrupt config files automatically.
- Do not change config format without updating `docs/CONFIG_FORMAT.md`.
- Do not add SFTP or split panes into early scope unless explicitly requested.
- Keep Windows/macOS portability in mind even when developing on Linux.

## Current config warning

DD-SSH currently stores secrets in plaintext under:

```text
secrets.mode = plain-v1
```

Treat all real `dd-ssh.json` files and backups as sensitive.

## Versioning rule

Every generated checkpoint must update:

```text
CMakeLists.txt → DD_SSH_VERSION_STRING
```

If appropriate, also update:

```text
README.md
docs/CHANGELOG.md
docs/TEST_MATRIX.md
Welcome/About phase text
```

## Architecture rule

Prefer focused classes.

Do not dump everything into MainWindow.

Current important classes:

- MainWindow
- ConnectDialog
- SettingsDialog
- WebTerminalTab
- TerminalBridge
- ConfigManager
- KnownHostsManager
- SshSession
- SshShellWorker

## Testing rule

Before declaring a terminal checkpoint healthy, manually test at least:

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

For lifecycle changes, also test remote reboot/disconnect and reconnect.


## Public alpha documentation

When changing user-facing behavior during the Andromeda alpha line, update the relevant docs:

- README.md
- docs/CHANGELOG.md
- docs/TEST_MATRIX.md
- docs/SECURITY_NOTES.md if secrets/config behavior changes
- docs/CONFIG_FORMAT.md if JSON structure changes
- docs/PUBLIC_ALPHA_CHECKLIST.md if release validation changes

Do not mark a test as PASS unless it was actually confirmed.
