# Build and Test — dev 0.1.8.3.1

Checkpoint: `dev 0.1.8.3.1` — SFTP logging polish.

## Build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Smoke test

- About shows `dev 0.1.8.3.1`.
- Diagnostic logging is OFF by default.
- Enable diagnostic logging in Settings.
- Upload a file that does not already exist remotely.
- Queue/upload a file that already exists remotely and approve overwrite / overwrite all.
- Confirm the log uses `SFTP upload preflight started` for overwrite checks and `SFTP file upload started` only for the actual transfer phase.
- Confirm queue, upload/download, folder transfer, Retry selected, Overwrite all / Skip all, terminal, and paste/Ctrl+C still work.

## Notes

This checkpoint is logging polish only. It does not change the transfer core, queue model, folder scan engine, terminal runtime, known-host handling, Windows KEX workaround, or config schema.
