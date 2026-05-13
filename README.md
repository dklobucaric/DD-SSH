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

Development checkpoint: `dev 0.1.3.4` — **Andromeda**.

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

`dev 0.1.2.0` added an experimental basic shell channel using libssh PTY + shell. `dev 0.1.2.1` cleaned up that temporary tab. `dev 0.1.2.3` added the first Qt WebEngine terminal tab with Ctrl+V/Paste button support. `dev 0.1.2.4` fixed the web terminal input dispatch path. `dev 0.1.2.5` added focus polish. `dev 0.1.3.0` introduced the first xterm.js-based renderer through Qt WebEngine. `dev 0.1.3.1` adds xterm.js FitAddon support and reports terminal columns/rows back to the SSH worker so libssh can request/change the remote PTY size. `dev 0.1.3.2.2` fixes the local bundled xterm.js/FitAddon Qt resource paths, so the terminal renderer no longer depends on a CDN at runtime. `dev 0.1.3.3` is the **Andromeda** terminal-app compatibility checkpoint: full-screen apps like `htop`, `nano`, `vim`, `top`, `mc`, and `tmux` can now be tested with local xterm.js, PTY resize, Ctrl-key input, and a local terminal reset action for recovery after visually messy full-screen app tests. `dev 0.1.3.4` makes the xterm.js terminal the default double-click action for saved sessions.


## dev 0.1.3.4

Double-click a saved session opens the xterm.js terminal by default. The saved-session auth test remains available from the session context menu.
