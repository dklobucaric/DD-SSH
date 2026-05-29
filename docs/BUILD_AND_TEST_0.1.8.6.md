# Build and Test — dev 0.1.8.6

Checkpoint: `dev 0.1.8.6` — File Manager delete UI polish.

## Build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Smoke test

- About shows `dev 0.1.8.6`.
- Saved-session context menu shows `Open File Manager`.
- Local panel shows `Delete local` instead of `Upload selected now`.
- Remote panel shows `Delete remote` instead of `Queue delete`.
- Queue upload/download still work.
- Remote delete from `dev 0.1.8.5` still works.
- Local delete queues a harmless local test file after confirmation.
- Start queue asks for destructive local delete confirmation.
- Confirmed local delete removes the local test file and refreshes the local panel.
- Diagnostic logging records local/remote delete events without secrets, private keys, clipboard contents, terminal I/O, or file contents.
