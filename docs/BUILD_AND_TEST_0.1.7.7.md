# Build and Test — dev 0.1.7.7

Checkpoint: **dev 0.1.7.7 — Single-file SFTP upload foundation**

## Build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Scope

This checkpoint adds one new transfer direction: upload one selected local file into the current remote SFTP folder. Download remains available.

## Smoke test

- About shows `dev 0.1.7.7`.
- File Manager opens with local and remote panels.
- Remote listing loads.
- Download selected still works.
- Select one local file and click `Upload selected`.
- Upload progress dialog appears.
- Remote panel refreshes and shows the uploaded file.
- Uploading the same filename again shows an overwrite prompt.
- Folder upload is blocked clearly.
- Terminal, paste, Ctrl+C, and terminal Session Traffic still work.

## Not expected yet

- Folder upload/download.
- Transfer queue.
- Delete/rename/chmod/mkdir.
- SFTP bytes in the live traffic monitor.
