# Build and test — dev 0.1.7.5

Checkpoint: `dev 0.1.7.5 — Local + remote read-only file manager foundation`

This checkpoint adds the first two-panel read-only File Manager foundation:

- local filesystem browser on the left
- remote SFTP browser on the right
- both panels are read-only
- no upload, download, delete, rename, queue, or progress/cancel behavior yet

The SSH terminal foundation should remain unchanged.

## Linux build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Expected smoke result

- Help → About shows `dev 0.1.7.5`
- Saved-session context menu shows `Open File Manager (read-only two-panel)`
- File Manager opens with local and remote panels side by side
- Local panel can browse folders with `Go`, `↑ Up`, `Refresh`, and double-click folder navigation
- Remote panel still performs known-host preflight and lists SFTP directories
- No transfer actions are available yet
- Existing terminal tabs, paste paths, Ctrl+C, logging, and terminal Session Traffic still work

## Windows/macOS smoke

After Linux passes, pull/build on Windows and macOS and repeat the UI smoke test. This checkpoint changes Qt Widgets UI and should be checked on all three platforms.
