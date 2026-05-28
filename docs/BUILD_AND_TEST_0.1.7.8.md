# Build and Test — dev 0.1.7.8

Checkpoint: **dev 0.1.7.8 — Transfer progress and cancel polish**

## Build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Scope

This checkpoint does **not** add a new transfer type. It polishes the existing one-file download and one-file upload actions.

Added/polished:

- transfer progress dialog shows transferred size, total size, percent, speed, and elapsed time
- download completion message shows formatted size, raw bytes, elapsed time, and average speed
- upload completion message shows formatted size, raw bytes, elapsed time, and average speed
- download cancel explains that the local target file was not replaced because DD-SSH uses a safe temporary file
- upload cancel explains that a partial remote file may remain on the server

## Smoke test

- About shows `dev 0.1.7.8`.
- File Manager opens with local and remote panels.
- Remote listing loads.
- Download selected still works.
- Download progress dialog shows progress, speed, and elapsed time.
- Download completion dialog shows size, raw bytes, elapsed time, and average speed.
- Cancelling a download shows a clear cancellation message.
- Upload selected still works.
- Upload progress dialog shows progress, speed, and elapsed time.
- Upload completion dialog shows size, raw bytes, elapsed time, and average speed.
- Cancelling an upload shows a clear cancellation message and warns about a possible partial remote file.
- Terminal, paste, Ctrl+C, and terminal Session Traffic still work.

## Not expected yet

- Folder upload/download.
- Transfer queue.
- Delete/rename/chmod/mkdir.
- SFTP bytes in the live traffic monitor.
- Overwrite dialog metadata comparison.
