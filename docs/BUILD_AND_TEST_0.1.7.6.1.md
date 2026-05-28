# Build and test — dev 0.1.7.6.1

Checkpoint: `dev 0.1.7.6.1 — Single-file SFTP download polish`

This checkpoint keeps the single-file remote download feature from `dev 0.1.7.6` and fixes two polish bugs found during large-file and cross-platform testing:

- remote SFTP `Size` column sorting now uses raw bytes instead of formatted display text
- completion message now reports `Downloaded: <formatted size> (<raw bytes> bytes)`

Still intentionally out of scope:

- upload
- folder transfer
- transfer queue
- sync engine
- SFTP traffic monitor integration
- richer overwrite comparison dialog with existing/new size and modified date

## Linux build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Smoke test

- Help → About shows `dev 0.1.7.6.1`
- File Manager opens with local and remote panels
- Remote listing still works
- Sort the remote `Size` column and verify mixed units sort by actual size, not text
- Download one small file
- Existing local target still triggers overwrite warning
- Completion dialog shows formatted size plus raw byte count, for example `Downloaded: 95 MB (99,614,720 bytes)`
- Attempting to download a remote folder is still refused
- SSH terminal tabs still open normally
- Paste, Ctrl+C, and terminal traffic monitor still work

## Cross-platform smoke

Run the same smoke test on:

- Linux
- Windows standalone/dev build
- macOS `.app` / DMG flow if needed
