## dev 0.1.4.5.1 — Andromeda

**Focus:** Config recovery actions.

- Renamed the safe continuation path to **Continue read-only**.
- Added **Restore latest valid backup** from the config recovery dialog.
- Added **Create fresh config**, which moves the corrupt file aside before writing a new empty default config.
- Removed the duplicate small settings-load warning for corrupt configs; Settings now opens the main recovery dialog instead.
- Corrupt configs are preserved as `dd-ssh.json.corrupt-<timestamp>`.

# Changelog

## dev 0.1.4.5 — Andromeda

**Focus:** Config recovery / corrupt JSON handling.

- Refuse to overwrite invalid or non-object `dd-ssh.json` files automatically.
- Added startup recovery warning for corrupt/unreadable config files.
- Recovery warning can open the config folder.
- Recovery warning lists available `dd-ssh.json.bak-*` backup files.
- The app can continue with default in-memory settings and an empty session list until the config file is fixed/restored/deleted.
- Known-host trust saves now also refuse to overwrite invalid config JSON.

# DD-SSH Changelog

This changelog tracks development checkpoints for DD-SSH.

The Welcome screen is intentionally kept as a short current-state dashboard. Detailed version history belongs here.

## Codename roadmap

```text
0.0.x — Launchpad / early prototype history
0.1.x — Andromeda / current MF 0.2 candidate line
0.2.x — Orion
0.3.x — Vega
0.4.x — Cassiopeia
1.0.x — Apollo
```

## dev 0.1.4.4 — Andromeda

**Focus:** App theme foundation.

- Added Settings → Appearance → App theme with `System default`, `Light`, and `Dark` options.
- Added `settings.appearance.app_theme` to `dd-ssh.json`.
- Applies the Qt app theme immediately after Settings are saved.
- Keeps the xterm.js terminal theme unchanged; this checkpoint only affects the main app chrome, dialogs, sidebar, menus, tabs, and status UI.
- Updated Welcome/README/config docs for app-level theming.

## dev 0.1.4.3 — Andromeda

**Focus:** Codename roadmap alignment.

- Aligned the Welcome screen and changelog codename roadmap with the current development reality: the 0.1.x line is now treated as Andromeda / MF 0.2 candidate work.
- Preserved Launchpad as historical early-prototype naming under 0.0.x instead of the active 0.1.x line.
- Kept SSH, session, settings, config backup, and xterm behavior unchanged.

## dev 0.1.4.2 — Andromeda

**Focus:** Config path casing and rotating backup policy implementation.

- Changed the application organization name from `DD-Lab` to `DD-LAB`, so new default Linux config path resolves to `~/.config/DD-LAB/DD-SSH/dd-ssh.json`.
- Implemented rotating `dd-ssh.json.bak-*` backups before config saves when backups are enabled in Settings.
- Backup retention now follows `settings.config_safety.max_backups`.
- Backup files are kept owner read/write only on Unix-like systems where Qt permissions apply.
- Updated the Settings dialog text to state that backup creation is active, not merely planned.
- Kept SSH/session/xterm behavior unchanged.

## dev 0.1.4.1 — Andromeda

**Focus:** Settings dialog sizing polish.

- Increased the default and minimum Settings dialog size so the first open is readable without manual resizing.
- Enabled the dialog size grip so the user can still resize it when desired.
- Added slightly better spacing and expanding form fields in the Settings dialog.
- Kept the plaintext secrets warning, config path display, toolbar Settings action, Tools → Settings action, and settings storage behavior unchanged.
- No SSH/session/xterm behavior changes.

## dev 0.1.4.0 — Andromeda

**Focus:** Settings foundation.

- Added a Settings dialog available from the toolbar and Tools menu.
- Settings are saved under the top-level `settings` block in `dd-ssh.json`.
- Added terminal font family and font size settings; changes apply to newly opened xterm.js terminal tabs.
- Added config safety settings for enabling backups and keeping the last N backups. This checkpoint stores the policy; rotating backup creation is planned next.
- Settings shows the active config file path and an explicit plaintext secrets warning.
- Kept SSH, session CRUD, xterm.js, PTY resize, reconnect, and lifecycle behavior unchanged.

## dev 0.1.3.9 — Andromeda

**Focus:** Andromeda test matrix documentation.

- Added `docs/TEST_MATRIX.md` as the validation checklist for the MF 0.2 / Real Terminal Foundation candidate.
- Captured confirmed tests for password login, private-key login, local xterm.js renderer, SSH PTY resize, terminal apps, lifecycle handling, remote reboot detection, and reconnect.
- Kept code behavior unchanged; this checkpoint documents the tested state before the final MF 0.2 stabilization pass.

## dev 0.1.3.8 — Andromeda

**Focus:** Terminal UI and status cleanup.

- Refined the xterm.js terminal header to show renderer, connection state, and PTY resize status more clearly.
- Replaced long local-action button labels with shorter labels and clearer tooltips.
- Added a consistent terminal status label showing state and target.
- Kept the underlying SSH/xterm lifecycle behavior unchanged after the reconnect checkpoint.

## dev 0.1.3.7 — Andromeda

**Focus:** Reconnect disconnected terminal.

- Added a Reconnect action to xterm.js terminal tabs after disconnect.
- Reconnect uses the same saved session and plaintext secret.
- Connected controls are restored after reconnect and remote input controls remain disabled while disconnected.
- This follows the remote-reboot lifecycle test from `dev 0.1.3.6`: DD-SSH can now fall down cleanly and stand back up from the same tab.

## dev 0.1.3.6 — Andromeda

**Focus:** Terminal lifecycle polish.

- Closing an active SSH terminal tab now asks before disconnecting and closing.
- Terminal tabs mark connected/disconnected state with simple title suffixes.
- Disconnect requests now surface in the terminal/status flow.
- xterm input is disabled after the SSH worker finishes.
- SSH read/write failures now break out of the worker loop instead of silently spinning.
- Remote reboot/disconnect testing should now show clearer connection-lost status messages.

## dev 0.1.3.5 — Andromeda

**Focus:** Welcome and changelog polish.

- Updated the Welcome screen from early skeleton wording to current project status.
- Added the codename roadmap to the Welcome screen.
- Added this `docs/CHANGELOG.md` file for detailed checkpoint history.
- Kept the About dialog as the source of truth for exact build identity.

## dev 0.1.3.4 — Andromeda

**Focus:** Make xterm terminal default for saved sessions.

- Double-clicking a saved session now opens the xterm.js terminal by default.
- Saved-session auth test remains available from the sidebar context menu.
- The app now behaves more like a real SSH client instead of an auth test harness.

## dev 0.1.3.3 — Andromeda

**Focus:** Terminal compatibility checkpoint and codename display.

- Added version codename support.
- Added milestone display to the About dialog.
- Added `Reset local terminal` action.
- Confirmed the Real Terminal Foundation direction for MF 0.2.

Confirmed working in this development line:

```text
- local bundled xterm.js renderer active
- password login
- private-key login
- PTY resize
- htop
- nano /tmp/dd-ssh-test.txt
- vim /tmp/dd-ssh-test.txt
- top
- clear
- stty size
```

## dev 0.1.3.2.2 — Andromeda

**Focus:** Local bundled xterm.js resource path fix.

- Fixed Qt resource path handling for bundled xterm.js assets.
- Confirmed `xterm.js ACTIVE - local bundled renderer` status.
- Removed practical dependency on CDN-hosted xterm.js for the terminal renderer.

## dev 0.1.3.2 — Andromeda

**Focus:** Bundle local xterm.js assets.

- Added local xterm.js, xterm.css, and FitAddon assets under `resources/xterm/`.
- Added third-party license notes for bundled terminal assets.
- Initial resource loading had path issues and was corrected in a later checkpoint.

## dev 0.1.3.1 — Andromeda

**Focus:** xterm.js FitAddon and SSH PTY resize sync.

- Added xterm.js fit handling.
- Added terminal size reporting from WebEngine to C++.
- Added SSH PTY resize handling through libssh.
- Verified that remote `stty size` changes when the DD-SSH window is resized.

## dev 0.1.3.0 — Andromeda

**Focus:** First xterm.js terminal renderer.

- Added the first xterm.js-based terminal renderer.
- Improved ANSI/color handling compared with fallback text rendering.
- Kept fallback renderer available for debugging.

## dev 0.1.2.x — Launchpad

**Focus:** Basic shell channel and web terminal bridge.

- Added a basic saved-session SSH shell channel.
- Added a temporary QWidget-based shell fallback.
- Added WebEngine/WebChannel terminal bridge experiments.
- Fixed terminal input and paste dispatch.
- Added focus polish for the web terminal.

## dev 0.1.1.x — Launchpad

**Focus:** Saved sessions and JSON config foundation.

- Added session save-to-JSON flow.
- Added plaintext portable secrets under `secrets.mode = "plain-v1"`.
- Added saved-session authentication from JSON.
- Added session edit/delete actions.
- Added duplicate host/user/port warning for manual saves.
- Preserved `known_hosts` separately from session CRUD.

## dev 0.1.0.x — Launchpad

**Focus:** Initial UI and SSH authentication foundation.

- Added Qt GUI skeleton.
- Added sidebar and tab layout.
- Added manual connection dialog.
- Added libssh handshake and authentication tests.
- Added known-host trust flow in `dd-ssh.json`.
