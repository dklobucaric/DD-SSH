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

Goal: Real Terminal Foundation and public-alpha readiness.

Already implemented:

- Saved sessions
- One-file JSON config
- Plaintext portable secrets
- known_hosts handling
- Password/private-key auth
- Session CRUD
- xterm.js local terminal
- PTY resize
- Terminal lifecycle/reconnect
- Settings foundation
- App light/dark/system theme
- Config backup/recovery
- Config import/export/restore
- Documentation/test matrix

Remaining before public alpha tag:

- Final public-alpha checklist pass
- Confirm latest test matrix on Linux and Windows
- Optional screenshots
- Known limitations review
- Decide whether to tag `dev-0.1.5.9` as an internal stabilization marker

Potential tag:

```text
v0.2.0-alpha — Andromeda
```

## 0.2.x — Orion

Focus: usability and session workflow polish.

Possible items:

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

`dev 0.1.6.4` adds repo hygiene, release-artifact rules, and checksum helpers while keeping SSH runtime behavior unchanged from `dev 0.1.6.3`. `dev 0.1.6.3` hardened the SSH trust chain so the real authentication/shell connection verifies the approved host key before sending secrets. `dev 0.1.6.2` added the first macOS Intel `.app` / `.dmg` deployment foundation. `dev 0.1.6.1.1` added README screenshots and a practical Debian packaging/install tutorial after the first `.deb` validation pass. `dev 0.1.6.1` began the packaging phase with the first Debian package experiment. `dev 0.1.5.9` is the stabilization docs and release polish checkpoint. It consolidates the 0.1.5.6 Windows standalone deployment pass, the 0.1.5.7 known-host multi-key portability fix, and the 0.1.5.8 Windows libssh KEX compatibility fix. The 0.1.5.x line prepares the repository for `v0.2.0-alpha — Andromeda` with public alpha docs, Windows Debug/Release/deploy-folder validation, release notes, known limitations, issue templates, cross-platform icon resources, WebEngine startup polish, and exit safety.

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

Next packaging polish candidates after this checkpoint:

```text
dev 0.1.6.5 — macOS DMG/dependency polish
dev 0.1.6.6 — Windows installer experiment
dev 0.1.6.7 — Linux package dependency polish
dev 0.1.6.8 — diagnostics and copy diagnostics
```

## Future file transport direction

DD-SSH may later add a simple file transport view for saved sessions. The intended model is a two-panel local/remote file manager using SFTP over libssh:

```text
left: local files
right: remote files
terminal tabs may remain open in the background
```

This must not be implemented before the terminal, packaging, diagnostics, and SSH trust-chain foundations are stable. Early code should avoid assuming that a saved session can only open a terminal. A saved session should be treated as a connection profile that may later open a terminal, diagnostics view, traffic monitor, or file transport view.
