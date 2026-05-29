# DD-SSH Roadmap

## Codename roadmap

```text
0.0.x — Launchpad / early prototype history
0.1.x — Andromeda / current MF 0.2 candidate line
0.2.x — Orion
0.3.x — Vega
0.4.x — Cassiopeia
1.0.x — Apollo
```

## Current line: 0.1.x — Andromeda

Goal: preserve the validated terminal foundation while building the File Transfer / File Manager track in small, testable checkpoints.

Confirmed baseline:

- `dev 0.1.7.1` is the closed terminal foundation / hardening checkpoint.
- The terminal baseline has been tested on Linux, Windows, and macOS.
- Saved sessions, xterm.js terminal, known-host multi-key portability, Windows KEX compatibility, diagnostic logging, Session Traffic, config import/export preview, packaging flows, and paste hardening must not regress.

Already implemented:

- Saved sessions
- One-file human-readable JSON config
- Plaintext portable secrets for alpha portability
- known_hosts handling with multi-key host entries
- Password/private-key auth
- Session CRUD
- xterm.js local terminal
- PTY resize
- Terminal lifecycle/reconnect
- Settings foundation
- App light/dark/system theme
- Config backup/recovery
- Config import/export/restore
- Optional diagnostic logging
- Session Traffic monitor
- Native paste hardening
- Linux/Windows/macOS tester packaging flows
- Documentation/test matrix

Active File Transfer / File Manager track:

Current checkpoint: `dev 0.1.8.4` — overwrite metadata dialog polish. This checkpoint keeps the accepted `dev 0.1.8.3.1` diagnostic logging baseline and improves upload/download overwrite prompts with existing and incoming file metadata.

```text
dev 0.1.7.2 — File transport architecture/design foundation [done]
dev 0.1.7.3 — SFTP connection proof of concept [done]
dev 0.1.7.4 — read-only remote file browser [superseded by 0.1.7.4.1]
dev 0.1.7.4.1 — read-only SFTP browser bugfix polish [passed]
dev 0.1.7.5 — local + remote two-panel read-only UI [done]
dev 0.1.7.6 — single-file download [superseded by 0.1.7.6.1]
dev 0.1.7.6.1 — single-file download polish [passed]
dev 0.1.7.7 — single-file upload [passed]
dev 0.1.7.8 — transfer progress/cancel polish [passed]
dev 0.1.8.0 — transfer queue foundation [passed]
dev 0.1.8.0.1 — transfer queue overwrite prompt polish [passed]
dev 0.1.8.0.2 — transfer queue overwrite-all polish [passed]
dev 0.1.8.0.3 — transfer queue retry-selected polish [passed]
dev 0.1.8.0.4.1 — transfer queue stabilization polish [passed]
dev 0.1.8.1 — folder transfer experiment [passed]
dev 0.1.8.1.1 — queue selected UI consolidation [passed]
dev 0.1.8.2 — file manager safety polish [done]
dev 0.1.8.3 — file transfer logging and diagnostics [accepted]
dev 0.1.8.3.1 — SFTP logging polish [accepted]
dev 0.1.8.4 — overwrite metadata dialog polish [current]
dev 0.1.8.5 — remote queue delete experiment [planned]
dev 0.1.8.6 — SFTP traffic monitor widget integration [planned]
dev 0.1.9.x — cross-platform file transfer stabilization
```

Target alpha milestone:

```text
v0.2.0-alpha — Terminal + basic file transfer
```

## 0.2.x — Orion

Focus: alpha stabilization after terminal + basic file transfer exists.

Possible items:

- File transfer stabilization after first alpha feedback
- Better session grouping UI
- Search/filter saved sessions
- Session detail panel
- Duplicate session action
- More polished session edit flow
- Better tab title/status UX
- More robust reconnect options
- Config import/export UX refinements

## 0.3.x — Vega

Focus: portability and config location.

Possible items:

- Custom config path picker
- Portable mode next to executable
- Config file reload detection
- External sync conflict detection
- Better backup browser/restore picker
- Import/export individual sessions

## 0.4.x — Cassiopeia

Focus: connection reliability and admin workflow.

Possible items:

- Keep-alive per session
- Default keep-alive setting
- Dead connection detection polish
- SSH agent investigation
- Keyboard-interactive auth polish
- Better known_hosts management UI

## 0.5.x — Multi-Exec foundation

Focus: controlled multi-target command sending.

Required behavior:

- Select active terminal tabs
- Preview targets
- Send text only
- Send text + Enter
- Dangerous command warnings
- Local multi-exec log without secrets

Multi-Exec is powerful and risky. It should not be rushed.

## 0.9.x — Advanced theming / polish

Possible items:

- Terminal theme picker
- Custom app themes
- Custom terminal colors
- Theme import/export
- Live terminal preference updates

Terminal theme customization is intentionally low priority compared with app-level usability.

## 1.0.x — Apollo

Goal: first serious public release.

Expected before Apollo:

- Linux/Windows/macOS validation
- Packaging story
- Installer or portable builds
- Security notes finalized
- Known limitations clearly documented
- Encrypted secret storage decision made
- No known data-loss bugs


## Current release-prep step

`dev 0.1.8.2` prepares the accepted file-manager and folder/queue baseline for third-party tester builds. It updates tester checklist, build/test notes, known limitations, README, About/Welcome text, and test matrix without changing transfer runtime behavior. `dev 0.1.8.1.1` consolidates File Manager queue buttons into two panel-level actions: local `Queue upload` and remote `Queue download`. Selected files are queued directly; selected folders are confirmed and recursively expanded into the existing queue. `dev 0.1.8.1` adds the first recursive folder-transfer experiment by scanning selected folders and expanding them into the existing sequential transfer queue. Destination folder creation is represented as queue items, and symlinks are skipped. `dev 0.1.8.0.4.1` stabilized the transfer queue before folder-transfer work by strengthening exit safety for running/pending queue work, locking queue/navigation controls while a queue run is active, and improving the no-pending-items message. `dev 0.1.8.0.3` adds Retry selected for finished queue items. `dev 0.1.8.0.2` adds Overwrite all / Skip all decisions for repeated queue overwrite conflicts. `dev 0.1.8.0.1` fixes queue overwrite prompt ordering so overwrite decisions are not hidden behind progress dialogs. `dev 0.1.8.0` added the first conservative transfer queue foundation for multiple individual file uploads/downloads, processed one at a time. `dev 0.1.7.8` polishes transfer progress, speed/elapsed reporting, completion summaries, and cancel feedback for both download and upload. `dev 0.1.7.7` added the first single-file local-to-remote upload action. `dev 0.1.7.6.1` polishes remote size sorting and the download completion message. `dev 0.1.7.6` added the first single-file SFTP download action on top of the two-panel File Manager. `dev 0.1.7.5` added the first two-panel read-only File Manager foundation with local and remote browsing side by side. `dev 0.1.7.4.1` polished the first read-only SFTP browser after Linux/macOS testing feedback. `dev 0.1.7.4` added the first graphical read-only remote SFTP browser. `dev 0.1.7.3` added the first saved-session SFTP probe using libssh SFTP, while still avoiding transfers. `dev 0.1.7.2` started the File Transfer / File Manager development track with architecture documentation and a harmless UI placeholder. `dev 0.1.7.1` remains the closed terminal foundation baseline after native paste hardening. `dev 0.1.7.0` hardened terminal transport by moving SSH output through a byte-stream/Base64/WebChannel path, using streaming UTF-8 decoding in xterm.js, and making terminal input writes partial-write aware. `dev 0.1.6.x` completed the logging, Session Traffic, config safety, macOS DMG, repo hygiene, packaging, and SSH trust-chain foundations. `dev 0.1.5.x` completed Windows standalone deployment, known-host multi-key portability, and Windows libssh KEX compatibility work.

---

## 0.1.5.x Windows/public-alpha preparation

```text
dev 0.1.5.0 — Public alpha release preparation
dev 0.1.5.1 — Windows build documentation and release build test
dev 0.1.5.2 — App icon integration
dev 0.1.5.3 — WebEngine startup polish
dev 0.1.5.4 — Windows deployment experiment
dev 0.1.5.5 — Exit safety and user guide polish
dev 0.1.5.6 — Windows standalone deployment test
dev 0.1.5.7 — Known-host multi-key portability polish
dev 0.1.5.8 — Windows libssh handshake compatibility polish
dev 0.1.5.9 — Stabilization docs and release polish
dev 0.1.6.1 — First Debian package experiment
dev 0.1.6.1.1 — README screenshots and Debian packaging tutorial polish
dev 0.1.6.2 — macOS Intel app/DMG foundation
dev 0.1.6.3 — SSH trust-chain hardening
dev 0.1.6.4 — Repo hygiene and release artifact workflow
dev 0.1.6.5 — macOS DMG/dependency polish
dev 0.1.6.6 — Optional diagnostic logging foundation
dev 0.1.6.7 — Basic Session Traffic Monitor
dev 0.1.6.8 — Config import/export safety preview
dev 0.1.6.9 — Bugfix stabilization
dev 0.1.7.0 — Terminal transport hardening
dev 0.1.7.1 — Native paste event hardening
dev 0.1.7.2 — File transport architecture/design foundation
```

Windows/public-alpha scope:

- document native Windows MSVC/Qt/vcpkg build
- test Release build separately from Debug
- document first-terminal startup delay from Qt WebEngine
- document RAM expectations caused by Qt WebEngine/Chromium
- integrate app/window/exe icons
- test `windeployqt` deployment folder
- copy vcpkg runtime DLLs into the deployment folder
- run deployed app outside the build environment
- collect bugfixes before `v0.2.0-alpha`

---

## 0.1.6.4 release-artifact policy

`dev 0.1.6.4` formalizes the repository rule:

```text
Git repository = source, docs, resources, packaging templates, scripts
GitHub Releases = generated packages and release artifacts
```

Generated `dist/`, build folders, `.deb`, `.dmg`, `.zip`, AppImage/MSI/package files, `.DS_Store`, `__MACOSX/`, and similar outputs must not be committed. Release assets should be uploaded with a generated `SHA256SUMS` file.

Checksum helpers:

```text
scripts/generate-checksums-linux.sh
scripts/generate-checksums-macos.sh
scripts/generate-checksums-windows.ps1
scripts/generate-checksums-windows.bat
```

## 0.1.6.5 macOS DMG/dependency polish

`dev 0.1.6.5` keeps runtime/SSH behavior unchanged and strengthens the macOS tester package path:

```text
- generate a deployable DD-SSH.app and Intel DMG
- keep the Applications shortcut in the DMG
- include README_FIRST.txt inside the DMG for unsigned-app/Gatekeeper guidance
- generate an otool dependency report beside the DMG
- warn about leftover /Users, /usr/local, or /opt/homebrew dependencies
- support STRICT_DEP_AUDIT=1 for fail-fast dependency checks on the build Mac
```

Next active feature track after the terminal foundation checkpoint:

```text
dev 0.1.7.2 — File transport architecture/design foundation [done]
dev 0.1.7.3 — SFTP connection proof of concept [done]
dev 0.1.7.4 — Read-only remote file browser [superseded by 0.1.7.4.1]
dev 0.1.7.4.1 — Read-only SFTP browser bugfix polish [passed]
dev 0.1.7.5 — Local + remote read-only file manager foundation [done]
dev 0.1.7.6 — Single-file download foundation [superseded by 0.1.7.6.1]
dev 0.1.7.6.1 — Single-file download polish [passed]
dev 0.1.7.7 — Single-file upload foundation [passed]
dev 0.1.7.8 — Transfer progress/cancel polish [passed]
dev 0.1.8.0 — Transfer queue foundation [done]
dev 0.1.8.0.1 — Transfer queue overwrite prompt polish [done]
dev 0.1.8.0.2 — Transfer queue overwrite-all polish [done]
dev 0.1.8.0.3 — Transfer queue retry-selected polish [passed]
dev 0.1.8.0.4.1 — Transfer queue stabilization polish [done]
dev 0.1.8.1 — Folder transfer experiment [passed]
dev 0.1.8.1.1 — Queue selected UI consolidation [passed]
dev 0.1.8.2 — File manager safety polish [done]
dev 0.1.8.3 — File transfer logging and diagnostics [accepted]
dev 0.1.8.3.1 — SFTP logging polish [accepted]
dev 0.1.8.4 — overwrite metadata dialog polish [current]
```

## 0.1.7.0 terminal transport hardening

`dev 0.1.7.0` keeps the UI and SSH trust model stable while improving terminal reliability:

```text
- carry SSH output as bytes instead of converting every read chunk directly to QString
- forward terminal bytes to WebEngine as Base64 over Qt WebChannel
- decode terminal output in JavaScript with TextDecoder(stream=true)
- queue terminal input as UTF-8 bytes
- handle partial ssh_channel_write() results so large paste/input is not silently truncated
- keep diagnostic logging free of terminal input/output content
```

## Active file transport direction

DD-SSH is now adding a simple file transport view for saved sessions. The intended model is a two-panel local/remote file manager using SFTP over libssh:

```text
left: local files
right: remote files
terminal tabs may remain open in the background
```

The terminal, packaging, diagnostics, and SSH trust-chain foundations are now stable enough to begin this work. Early code must avoid assuming that a saved session can only open a terminal. A saved session should be treated as a connection profile that may open a terminal, file manager, diagnostics view, or future traffic/transport view.


## 0.1.7.2 file transport architecture/design foundation

`dev 0.1.7.2` starts the File Transfer / File Manager development track without opening an SFTP runtime yet.

Scope:

- update app version to `dev 0.1.7.2`
- add `docs/FILE_TRANSFER_ARCHITECTURE.md`
- add a saved-session context menu placeholder: `Open File Manager (planned)`
- update README, changelog, test matrix, build/test docs, and testcase docs
- preserve `dev 0.1.7.1` as the terminal regression baseline

Explicit non-goals:

- no SFTP connection
- no file listing
- no upload/download
- no transfer progress UI
- no terminal transport changes
- no known-host behavior changes
- no config schema migration

## 0.1.7.1 native paste event hardening

`dev 0.1.7.1` is a focused bugfix checkpoint after terminal transport hardening. It keeps the byte-stream/Base64 terminal transport from `dev 0.1.7.0`, but routes right-click paste, Ctrl+Shift+V, and macOS Command+V through the same DD-SSH safe paste flow used by the toolbar Paste button. The goal is to avoid raw bracketed paste markers such as `^[[200~` / `^[[201~` being sent to the remote shell.
