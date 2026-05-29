# Build and Test — dev 0.1.8.4

Checkpoint: `dev 0.1.8.4` — Overwrite metadata dialog polish.

## Build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Smoke test

- About shows `dev 0.1.8.4`.
- Immediate download of an already-existing local file shows existing local metadata and incoming remote metadata.
- Immediate upload of an already-existing remote file shows existing remote metadata and incoming local metadata.
- Queue download conflict shows local target metadata and incoming remote metadata.
- Queue upload conflict shows remote target metadata and incoming local metadata.
- Overwrite, Skip, Overwrite all, Skip all, and Cancel queue still behave as before.
- Diagnostic logging behavior from `dev 0.1.8.3.1` still works and remains OFF by default.
- Terminal, paste/Ctrl+C, and normal file transfer still work.

## Notes

This checkpoint changes overwrite dialog text and metadata only. It does not change the transfer core, queue model, folder scan engine, SSH/terminal runtime, known-host handling, Windows KEX workaround, or config schema.
