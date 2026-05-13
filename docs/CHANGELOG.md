# DD-SSH Changelog

This changelog tracks development checkpoints for DD-SSH.

The Welcome screen is intentionally kept as a short current-state dashboard. Detailed version history belongs here.

## Codename roadmap

```text
0.1.x — Launchpad
0.2.x — Andromeda
0.3.x — Orion
0.4.x — Vega
0.5.x — Cassiopeia
1.0.x — Apollo
```

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
