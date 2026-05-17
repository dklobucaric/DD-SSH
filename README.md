# DD-SSH

**DD-SSH** is a clean cross-platform SSH client and session manager.

The goal is to build a lightweight SSH tool for Linux, Windows, and macOS with:

- SSH terminal
- Session manager
- Password and private-key authentication
- Tabs
- Copy/paste
- Single JSON config
- Export/import
- Sync-friendly config
- Keep-alive
- Known-host handling
- Multi-exec command

Early versions intentionally do **not** include SFTP, split-screen, cloud accounts, telemetry, or AI features inside the app.

## Project status

Development checkpoint: `dev 0.1.4.7` — **Andromeda**.

Current working foundation:

- Qt GUI skeleton with sidebar, tabs, toolbar, and About dialog
- Manual SSH connection dialog
- libssh handshake and password/private-key authentication tests
- known-host trust flow stored in `dd-ssh.json`
- Saved sessions loaded from `dd-ssh.json` into the sidebar
- Portable plaintext `secrets.mode = "plain-v1"` for early v1.0.x development
- Connect/auth test from saved sessions
- Delete saved sessions while preserving known-host trust records
- Edit saved sessions while keeping or replacing the saved plaintext secret
- Duplicate target warning when saving `username@host:port` that already exists
- Experimental basic saved-session SSH shell channel from the sidebar context menu
- First xterm.js-based web terminal renderer, now with local bundled assets, FitAddon, SSH PTY resize sync, and terminal-app compatibility polish
- Double-clicking a saved session now opens the xterm.js terminal by default; the saved-session auth test remains available from the context menu
- Terminal lifecycle polish: active tab close confirmation, connected/disconnected tab markers, disconnect cleanup, clearer remote disconnect/reboot status handling, reconnect from disconnected xterm.js tabs, and clearer terminal UI/status labels
- Andromeda test matrix documentation under `docs/TEST_MATRIX.md`
- Settings foundation: terminal font settings, config path visibility, plaintext secrets warning, rotating config-backup policy, improved initial dialog sizing, and app appearance theme selection
- Linux default config path now uses `~/.config/DD-LAB/DD-SSH/dd-ssh.json`
- Config recovery guard: invalid/corrupt `dd-ssh.json` is not overwritten automatically; startup warns, lists available backups, and can open the config folder

Saved session management is a real DD-SSH feature direction, not just a test helper. The long-term goal is full session CRUD: create/save, load/connect, edit/update, delete, and later import/export from one portable JSON file.

## Technology direction

- C++
- Qt 6
- CMake
- libssh
- xterm.js via Qt WebEngine for early terminal frontend
- JSON config

## Documentation

Start here:

- [Project Blueprint](docs/PROJECT_BLUEPRINT.md)
- [Roadmap](docs/ROADMAP.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Config Format](docs/CONFIG_FORMAT.md)
- [Building](docs/BUILDING.md)

## Current shell limitation

`dev 0.1.2.0` added an experimental basic shell channel using libssh PTY + shell. `dev 0.1.2.1` cleaned up that temporary tab. `dev 0.1.2.3` added the first Qt WebEngine terminal tab with Ctrl+V/Paste button support. `dev 0.1.2.4` fixed the web terminal input dispatch path. `dev 0.1.2.5` added focus polish. `dev 0.1.3.0` introduced the first xterm.js-based renderer through Qt WebEngine. `dev 0.1.3.1` adds xterm.js FitAddon support and reports terminal columns/rows back to the SSH worker so libssh can request/change the remote PTY size. `dev 0.1.3.2.2` fixes the local bundled xterm.js/FitAddon Qt resource paths, so the terminal renderer no longer depends on a CDN at runtime. `dev 0.1.3.3` is the **Andromeda** terminal-app compatibility checkpoint: full-screen apps like `htop`, `nano`, `vim`, `top`, `mc`, and `tmux` can now be tested with local xterm.js, PTY resize, Ctrl-key input, and a local terminal reset action for recovery after visually messy full-screen app tests. `dev 0.1.3.4` makes the xterm.js terminal the default double-click action for saved sessions. `dev 0.1.3.6` adds terminal lifecycle polish: connected/disconnected tab markers, safer active-tab close confirmation, disconnect cleanup, and clearer status when the remote shell closes or the SSH transport drops. `dev 0.1.3.7` adds a Reconnect action to disconnected xterm.js terminal tabs so a dropped/rebooted session can be started again from the same tab using the same saved session. `dev 0.1.3.8` cleans up the terminal UI/status surface: clearer renderer/state header text, shorter terminal action labels, improved tooltips, and status labels that show the current state and target. `dev 0.1.3.9` adds the Andromeda test matrix document so MF 0.2 validation is tracked in the repository. `dev 0.1.4.0` starts the Settings foundation: settings are stored under the top-level `settings` block in `dd-ssh.json`, terminal font settings apply to newly opened xterm.js tabs, and config backup policy is stored for the next safety checkpoint. `dev 0.1.4.1` polishes the Settings dialog initial size and layout so it opens readable without manual resizing. `dev 0.1.4.2` changes the application organization casing to `DD-LAB` for new default config paths and implements rotating config backups before saves when enabled in Settings. `dev 0.1.4.3` aligns the codename roadmap so the active 0.1.x line is documented as Andromeda / MF 0.2 candidate work, while Launchpad is kept as historical early-prototype naming. `dev 0.1.4.4` adds the first app-level appearance foundation: Settings can store System, Light, or Dark theme preference and apply the Qt app theme immediately after saving. The xterm.js terminal theme is intentionally unchanged for now. `dev 0.1.4.5` adds config recovery handling: invalid or non-object `dd-ssh.json` files are never overwritten automatically, startup shows a recovery warning, available backups are listed, and the config folder can be opened directly from the warning. `dev 0.1.4.5.1` adds explicit recovery actions: continue read-only, restore the latest valid backup, or move the corrupt file aside and create a fresh empty config. `dev 0.1.4.6` polishes the menu/dialog UX: File is reserved for app/config actions and currently only contains Exit, while Session owns New Session, Connect / Auth test, and Edit selected session. The shared ConnectDialog now has explicit Manual Connect, New Saved Session, and Edit Saved Session modes. `dev 0.1.4.7` cleans up the main window by hiding the quick action toolbar by default and adding a Settings option to show it again when desired.


## dev 0.1.3.4

Double-click a saved session opens the xterm.js terminal by default. The saved-session auth test remains available from the session context menu.


## Current development status

DD-SSH is currently in the **Andromeda / MF 0.2 candidate** development line. The current focus is the Real Terminal Foundation: local bundled xterm.js renderer, SSH PTY resize, saved sessions, password/private-key login, and terminal app compatibility.

See [`docs/CHANGELOG.md`](docs/CHANGELOG.md) for detailed checkpoint history.
See [`docs/TEST_MATRIX.md`](docs/TEST_MATRIX.md) for the current Andromeda test checklist.
