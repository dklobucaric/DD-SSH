# Build and Test — dev 0.1.8.0.3

Checkpoint: `dev 0.1.8.0.3` — Transfer queue retry-selected polish.

This checkpoint keeps the existing transfer queue foundation and adds one small usability action: `Retry selected`.

## Scope

Included:

- `Retry selected` button in the File Manager transfer queue controls.
- Selected `Done`, `Failed`, `Cancelled`, or `Skipped` queue items can be moved back to `Pending`.
- Requeued items preserve direction, source path, target path, display name, and size.
- Existing `Start queue` processes retried items through the same transfer and overwrite decision paths.

Not included:

- Folder transfer.
- Parallel transfers.
- Resume support.
- Drag/drop queue ordering.
- Automatic retry-all engine.
- SFTP Session Traffic integration.
- Delete/rename/chmod/mkdir.

## Linux build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Windows build

Use the existing Windows build flow from `docs/WINDOWS_BUILD.md`.

Smoke target:

- app launches
- About shows `dev 0.1.8.0.3`
- File Manager opens
- queue retry selected flow works

## macOS build

Use the existing macOS build flow from `docs/MACOS_BUILD.md`.

Smoke target:

- app launches
- About shows `dev 0.1.8.0.3`
- File Manager opens
- queue retry selected flow works

## Regression focus

Validate that this checkpoint does not regress:

- immediate `Download selected now`
- immediate `Upload selected now`
- queue overwrite/skip/overwrite-all/skip-all behavior
- queue cancel behavior
- terminal xterm.js startup
- paste / Ctrl+C
- terminal-only Session Traffic indicator
