# DD-SSH Changelog

## dev 0.1.5.7 — Andromeda

Known-host multi-key portability polish.

- Updated the project identity to `dev 0.1.5.7`.
- Changed known-host storage from one key per `host:port` to multi-key storage under `keys`.
- Preserved compatibility with the old `algorithm` / `fingerprint` known-host format and migrates it on the next save.
- Added a separate **Additional SSH host key** decision path with **Trust additional key**, **Trust once**, and **Cancel**.
- Kept the strong **SSH host key changed** warning for true same-key-type fingerprint mismatches.
- Fixed the saved-session shell known-host decision flow so **Trust once** can continue the current shell-open attempt.
- Replaced `scripts/windows-deploy-release.bat` with the simpler working BAT validated during the Windows standalone deployment test.
- Added a dedicated known-host portability regression test note for the Windows 10 ECDSA vs Linux/Windows 11 ED25519 case.

## dev 0.1.5.6 — Andromeda

Windows standalone deployment test.

- Updated the project identity to `dev 0.1.5.6`.
- Hardened `scripts/windows-deploy-release.bat` for the standalone Windows release-folder test.
- Added deployment sanity checks for the deployed executable, Qt platform plugin, Qt WebEngine runtime hints, libssh, OpenSSL, and zlib DLLs.
- Updated Windows deployment documentation for `dist\windows-release`, no-manual-`PATH` launch testing, and clean Windows 10 validation.
- Updated README, About/Welcome text, roadmap, known limitations, public alpha checklist, build docs, packaging notes, and test matrix for the 0.1.5.6 focus.
- Kept SSH authentication, terminal runtime, config import/export/recovery, icon resources, and exit-safety behavior unchanged.

## dev 0.1.5.5 — Andromeda

Exit safety and user guide polish.

- Added application-level exit safety for active SSH terminal sessions.
- Closing the main window with active SSH connections now asks before disconnecting them.
- `File → Exit` now uses the same close protection as the window close button.
- Added user-facing documentation explaining that new sessions are saved only after successful SSH authentication.
- Updated README, user guide, troubleshooting, feature list, test matrix, and release-prep notes.
- Kept SSH authentication, xterm.js terminal runtime, config import/export, icons, and Windows deployment logic unchanged.

## dev 0.1.5.4 — Andromeda

Windows deployment experiment.

- Added `docs/WINDOWS_DEPLOYMENT.md` with the first `windeployqt`-based deployment workflow.
- Added `scripts/windows-deploy-release.bat` as an experimental helper for creating a standalone Windows release folder.
- Documented copying vcpkg runtime DLLs such as libssh/OpenSSL/zlib into the deployment folder.
- Documented testing the deployed app outside the build tree and without Qt/vcpkg PATH setup.
- Updated Windows build/release docs, roadmap, test matrix, and public-alpha checklist for deployment validation.
- Kept SSH/session/terminal runtime behavior and the updated icon resources unchanged.

## dev 0.1.5.3 — Andromeda

WebEngine startup polish.

- Added a visible terminal startup notice while the local xterm.js / Qt WebEngine renderer is preparing.
- Added clearer UI states for preparing/loading/ready/failed terminal renderer startup.
- Documented the Windows first-terminal startup delay as expected Qt WebEngine initialization behavior.
- Kept xterm.js terminal behavior, SSH/session logic, and icon resources otherwise unchanged.

## dev 0.1.5.2 — Andromeda

App icon integration.

- Added cross-platform icon resources generated from the uploaded DD-SSH master PNG.
- Added Qt resource icons under `:/icons/`.
- Set the Qt application/window icon from `:/icons/dd-ssh.png`.
- Added Windows `.ico` and `.rc` resources for the `.exe` icon.
- Added Linux PNG icon size variants for future desktop packaging.
- Added macOS `.iconset` and `.icns` prep for future app bundle packaging.
- Updated CMake integration for Windows and macOS icon resources.
- No SSH/session/terminal runtime logic changes are intended in this checkpoint.

## dev 0.1.5.1 — Andromeda

Windows build documentation and release build test preparation.

- Documented the first native Windows build path using MSVC x64, CMake, Ninja, Qt 6.11.1 MSVC 2022 64-bit, Qt WebEngine/WebChannel/Positioning, vcpkg `libssh`, and vcpkg `pkgconf`.
- Added `docs/WINDOWS_BUILD.md` with Debug and Release build commands.
- Documented Windows-specific observations: first WebEngine terminal startup delay, higher RAM usage from Qt WebEngine/Chromium, Qt cache folder, and AppData config path.
- Updated README, Welcome screen, Building docs, Test Matrix, Roadmap, and Public Alpha Checklist with Windows validation status.
- Added `.gitignore` coverage for Windows/MSVC local build artifacts.
- No SSH/session/terminal runtime logic changes are intended in this checkpoint.

## dev 0.1.5.0 — Andromeda

Public alpha release preparation.

- Added `docs/PUBLIC_ALPHA_CHECKLIST.md` as the final pre-alpha gate.
- Added `docs/RELEASE_NOTES_v0.2.0-alpha.md` draft.
- Added `docs/KNOWN_LIMITATIONS.md` for public tester visibility.
- Added GitHub issue templates for bug reports, terminal issues, config/recovery issues, and feature requests.
- Added pull request template with project safety checklist.
- Updated README, Welcome screen, and test matrix for public alpha preparation.

## dev 0.1.4.9 — Andromeda

**Focus:** Public alpha documentation pass.

- Rebuilt README as a public-alpha oriented project entry point.
- Added comprehensive documentation index.
- Added Getting Started guide.
- Added User Guide.
- Added Use Cases document.
- Added Features and Limitations document.
- Added Config Management document.
- Added Troubleshooting document.
- Expanded Architecture, Config Format, Security Notes, Roadmap, Building, Packaging, Release Process, and Test Matrix docs.
- Updated Welcome/About phase text to documentation pass.
- No SSH/session/terminal runtime logic changes are intended in this checkpoint.

## dev 0.1.4.8 — Andromeda

**Focus:** Config import/export/restore polish.

- Added File-menu config actions: Open Config Folder, Export Config, Import Config, Restore Latest Backup, and Exit.
- Export Config copies the active `dd-ssh.json` to a user-selected path.
- Import Config validates the selected JSON file, warns before replacement, creates a pre-import backup, then reloads settings and saved sessions.
- Restore Latest Backup restores the newest valid `dd-ssh.json.bak-*` backup and moves the previous active config aside as `dd-ssh.json.pre-restore-*`.

## dev 0.1.4.7 — Andromeda

**Focus:** Quick toolbar visibility polish.

- Hides quick action toolbar by default.
- Adds Settings option to show/hide quick action toolbar.
- Persists toolbar setting under `settings.behavior.show_quick_toolbar`.

## dev 0.1.4.6 — Andromeda

**Focus:** Session menu and dialog mode polish.

- File menu reserved for app/config actions.
- Session menu owns New Session, Connect / Auth test, and Edit selected session.
- ConnectDialog has Manual Connect, New Saved Session, and Edit Saved Session modes.

## dev 0.1.4.5.1 — Andromeda

**Focus:** Config recovery actions.

- Added Continue read-only.
- Added Restore latest valid backup.
- Added Create fresh config.
- Corrupt config files are preserved as `.corrupt-*`.

## dev 0.1.4.5 — Andromeda

**Focus:** Config recovery guard.

- Invalid/non-object `dd-ssh.json` is not overwritten automatically.
- Startup recovery warning lists backups and can open config folder.

## dev 0.1.4.4 — Andromeda

**Focus:** App theme foundation.

- Added app appearance setting: System / Light / Dark.
- Theme applies to Qt app chrome, not xterm.js terminal.

## dev 0.1.4.3 — Andromeda

**Focus:** Codename roadmap alignment.

- Reframed 0.1.x as Andromeda / MF 0.2 candidate line.
- Kept Launchpad as 0.0.x early-prototype history.

## dev 0.1.4.2 — Andromeda

**Focus:** Config path and backup policy implementation.

- Linux path casing changed to `DD-LAB/DD-SSH`.
- Implemented rotating config backups before saves.

## dev 0.1.4.1 — Andromeda

**Focus:** Settings dialog sizing polish.

- Settings dialog opens at a readable default size.

## dev 0.1.4.0 — Andromeda

**Focus:** Settings foundation.

- Added Settings dialog.
- Added terminal font settings.
- Added config path display.
- Added backup policy settings.
- Added plaintext secrets warning.

## dev 0.1.3.9 — Andromeda

**Focus:** Andromeda test matrix documentation.

- Added test matrix for Real Terminal Foundation validation.

## dev 0.1.3.8 — Andromeda

**Focus:** Terminal UI and status cleanup.

- Improved terminal header/state labels.
- Shortened terminal action labels.
- Improved status feedback.

## dev 0.1.3.7 — Andromeda

**Focus:** Reconnect disconnected terminal.

- Added Reconnect action for disconnected xterm.js terminal tabs.

## dev 0.1.3.6 — Andromeda

**Focus:** Terminal lifecycle polish.

- Added close confirmation for active SSH tabs.
- Added connected/disconnected tab markers.
- Improved remote disconnect/reboot handling.

## dev 0.1.3.5 — Andromeda

**Focus:** Welcome and changelog polish.

- Updated Welcome screen.
- Added codename roadmap.

## dev 0.1.3.4 — Andromeda

**Focus:** Double-click opens terminal.

- Saved session double-click opens xterm.js terminal by default.
- Auth test remains in context menu.

## dev 0.1.3.3 — Andromeda

**Focus:** Terminal compatibility checkpoint.

- Added codename/milestone display.
- Added reset local terminal action.
- Confirmed terminal app compatibility direction.

## dev 0.1.3.2.2 — Andromeda

**Focus:** Local xterm resource path fix.

- Fixed Qt resource path so bundled xterm.js/FitAddon load locally.

## dev 0.1.3.1 — Andromeda

**Focus:** xterm fit + SSH PTY resize.

- Added FitAddon.
- Added terminal resize reporting to SSH worker.
- Remote `stty size` follows window size.

## dev 0.1.3.0 — Andromeda

**Focus:** First xterm.js terminal renderer.

- Added first xterm.js renderer path through Qt WebEngine.

## dev 0.1.2.x — Launchpad-to-Andromeda bridge

- Added basic saved-session SSH shell channel.
- Added web terminal fallback.
- Added paste/input dispatch fixes.
- Added focus polish.

## dev 0.1.1.x — Launchpad

- Added saved sessions.
- Added plaintext secrets.
- Added connect from saved session.
- Added edit/delete session.
- Added duplicate target warning.

## dev 0.1.0.x — Launchpad

- Initial Qt GUI skeleton.
- Manual connection dialog.
- SSH handshake/auth tests.
- known_hosts storage.

## Codename roadmap

```text
0.0.x — Launchpad / early prototype history
0.1.x — Andromeda / current MF 0.2 candidate line
0.2.x — Orion
0.3.x — Vega
0.4.x — Cassiopeia
1.0.x — Apollo
```
