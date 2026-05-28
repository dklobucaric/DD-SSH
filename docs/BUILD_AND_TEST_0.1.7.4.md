# Build and test — dev 0.1.7.4

## Checkpoint

`dev 0.1.7.4 — Read-only remote file browser`

This checkpoint turns the previous SFTP proof-of-concept into the first graphical read-only remote browser tab.

It intentionally does not implement upload, download, delete, rename, queue, recursive transfer, or a local file panel yet.

## Linux build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Expected app version

Help → About DD-SSH should show:

```text
Version: dev 0.1.7.4
Codename: Andromeda
Milestone: Read-only remote file browser
```

## Basic smoke test

1. Start DD-SSH.
2. Confirm saved sessions load in the sidebar.
3. Double-click a saved session and confirm the existing xterm.js terminal still opens.
4. Right-click a saved session.
5. Choose `Open File Manager (read-only)`.
6. Confirm the known-host prompt still appears for unknown/additional/changed hosts.
7. Confirm a new tab opens with a remote path bar, `Go`, `Up`, `Refresh`, and a table.
8. Confirm the table lists remote entries with columns:
   - Name
   - Type
   - Size
   - Modified
   - Permissions
9. Double-click a remote directory and confirm the browser navigates into it.
10. Click `Up` and confirm it navigates to the parent path.
11. Type a remote path manually and click `Go`.
12. Click `Refresh`.

## Regression checks

The following should still work exactly as before:

- xterm.js terminal opens from double-click
- right-click paste
- Ctrl+Shift+V / Command+V paste
- toolbar Paste
- Ctrl+C remote interrupt
- optional diagnostic logging
- Session Traffic status-bar indicator for terminal tabs
- config import/export/restore preview
- known-host multi-key trust flow
- Windows-safe libssh KEX compatibility path

## Notes

The first read-only browser implementation is intentionally simple and may block briefly while listing a directory. A worker-backed browser can be introduced later when transfers/progress/cancel become necessary.

The browser uses libssh SFTP APIs through the existing SFTP listing path. It does not use shell commands such as `ls`, `cat`, `scp`, or `base64`.
