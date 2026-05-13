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

Development checkpoint: `dev 0.1.3.1`.

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
- First xterm.js-based web terminal renderer, now with FitAddon and SSH PTY resize sync

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

`dev 0.1.2.0` added an experimental basic shell channel using libssh PTY + shell. `dev 0.1.2.1` cleaned up that temporary tab. `dev 0.1.2.3` added the first Qt WebEngine terminal tab with Ctrl+V/Paste button support. `dev 0.1.2.4` fixed the web terminal input dispatch path. `dev 0.1.2.5` added focus polish. `dev 0.1.3.0` introduced the first xterm.js-based renderer through Qt WebEngine. `dev 0.1.3.1` adds xterm.js FitAddon support and reports terminal columns/rows back to the SSH worker so libssh can request/change the remote PTY size. xterm.js and FitAddon are still loaded from a CDN for this development checkpoint, with the previous fallback renderer used if assets cannot be loaded. Local bundled xterm.js assets are planned next. Full-screen apps like `htop`, `nano`, `vim`, `mc`, and `tmux` can now be tested, but may still need keyboard/input polish before they are considered supported.
