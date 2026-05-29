# Build and Test — dev 0.1.8.2

Checkpoint: `dev 0.1.8.2` — File manager safety polish.

This checkpoint improves tester-facing safety around the File Manager queue and experimental folder transfer. It does not introduce a new transfer engine.

## Build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Expected smoke result

- About shows `dev 0.1.8.2`.
- Terminal sessions still open and paste/Ctrl+C still work.
- File Manager opens with local `Queue upload` and remote `Queue download`.
- Single-file upload/download still work.
- Queue upload/download, Retry selected, Overwrite all, Skip all, and folder queue still work.

## Safety-polish checks

- Folder queue confirmation clearly explains the experimental behavior and safety rules.
- Folder queue scan summary appears after queuing a folder.
- Summary shows direct files, folder files, folder create items, skipped/cancelled items, and large-folder warnings when relevant.
- Queue completion refreshes local and remote panels.
- Path-related failures show a useful path in the queue item message.
