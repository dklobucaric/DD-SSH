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

- Final README/docs polish
- Confirm latest test matrix
- Optional screenshots
- Known limitations review
- GitHub issue template maybe

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

`dev 0.1.5.2` is the app icon integration checkpoint. The 0.1.5.x line prepares the repository for `v0.2.0-alpha — Andromeda` with public alpha docs, Windows build validation, release notes, known limitations, issue templates, and cross-platform icon resources.


---

## 0.1.5.x Windows/public-alpha preparation

```text
dev 0.1.5.0 — Public alpha release preparation
dev 0.1.5.1 — Windows build documentation and release build test
dev 0.1.5.2 — App icon integration
dev 0.1.5.3 — WebEngine startup polish
dev 0.1.5.4 — Windows deployment experiment
```

Planned Windows-related scope:

- document native Windows MSVC/Qt/vcpkg build
- test Release build separately from Debug
- document first-terminal startup delay from Qt WebEngine
- document RAM expectations caused by Qt WebEngine/Chromium
- app/window/exe icons added in 0.1.5.2
- later test `windeployqt` and standalone deployment folder
