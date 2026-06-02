# Feature audit — dev 0.1.8.7

Checkpoint: `dev 0.1.8.7` — Release/tester packaging polish.

This document is a tester-facing inventory of what DD-SSH currently does and what it intentionally does not do yet.

## Core session and config

- Saved sessions loaded from one portable `dd-ssh.json`.
- Human-readable JSON config remains the active design through the current alpha line.
- `plain-v1` secrets are supported for early portability; they are not encrypted.
- Password and private-key authentication.
- Known-host trust handling before authentication.
- Multi-key host-key storage per `host:port`, useful when platforms negotiate different legitimate host-key algorithms.
- New sessions are saved only after a successful auth test.
- Config import/export/restore and corrupt-config recovery.
- Optional diagnostic logging, OFF by default.

## Terminal

- xterm.js terminal renderer with bundled local assets.
- SSH PTY resize sync.
- Terminal lifecycle/disconnect handling.
- Tested terminal apps include `htop`, `nano`, `vim`, `top`, and `clear`.
- Toolbar Paste, right-click paste, Ctrl+Shift+V, and macOS Command+V use the safe paste path.
- Multiline paste into full-screen editors uses terminal-friendly newlines so YAML/config text keeps line structure in `nano`.
- Ctrl+C remote interrupt behavior.
- Status-bar Session Traffic for active terminal sessions.

## File Manager alpha

- Two-panel local/remote File Manager.
- Remote browsing uses libssh SFTP APIs.
- Local panel: `Queue upload`, `Delete local`.
- Remote panel: `Queue download`, `Delete remote`.
- Sequential transfer queue, one item at a time.
- Single files and selected folders can be expanded into queue items.
- Experimental recursive folder upload/download uses scan -> create-dir items -> file-transfer items.
- Queue statuses: Pending, Running, Done, Failed, Cancelled, Skipped.
- Retry selected for finished/skipped/cancelled/failed items.
- Remove selected and Clear finished.
- Overwrite, Skip, Overwrite all, Skip all.
- Overwrite metadata dialogs show existing/incoming path, size, and modified time where available.
- Local and remote queued delete support regular files, symlinks, and empty directories.
- Delete confirmations are destructive and appear when the queue reaches the delete item, preserving queue order.
- Queue completion refreshes local and remote panels.
- SFTP/file-transfer diagnostic logs are emitted only when logging is enabled.

## Packaging/tester flow

- Linux Release build helper.
- Debian package helper.
- macOS Intel `.app` / `.dmg` helper with otool dependency audit report.
- Windows portable deployment helper.
- Windows portable ZIP helper for tester builds.
- SHA256 checksum helpers for Linux/macOS/Windows release assets.

## Intentional current limitations

- No sync/mirror mode.
- No partial-transfer resume.
- No parallel transfers.
- No recursive non-empty folder delete.
- No drag/drop.
- No chmod/chown/rename yet.
- No permission or timestamp preservation yet.
- No symlink following in recursive folder transfer.
- SFTP bytes are not yet included in the status-bar Session Traffic widget.
- No encrypted/master-password secrets yet; `plain-v1` is still the early alpha portability mode.

## Tester advice

Use harmless test folders first. Do not test destructive delete actions on important production paths. Keep `dd-ssh.json` private because current alpha secrets are human-readable.
