# Build and Test — dev 0.1.8.0

Checkpoint: **dev 0.1.8.0 — Transfer queue foundation**

## Build

```bash
cd ~/DD-SSH
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
./build/dd-ssh
```

## Scope

This checkpoint adds the first transfer queue foundation on top of the existing single-file download/upload paths.

Added:

- `Queue download(s)` for multiple selected remote files
- `Queue upload(s)` for multiple selected local files
- queue table with Status / Direction / Name / Size / Source / Target
- `Start queue`, `Remove selected`, and `Clear finished`
- sequential one-file-at-a-time execution
- queue item statuses: Pending / Running / Done / Failed / Cancelled / Skipped

Preserved:

- `Download selected now` still performs one immediate download
- `Upload selected now` still performs one immediate upload
- existing progress, speed, elapsed, completion, and cancel feedback is reused
- folder transfer remains blocked

## Smoke test

- About shows `dev 0.1.8.0`.
- File Manager opens with local and remote panels.
- Remote SFTP listing loads.
- Existing immediate download works.
- Existing immediate upload works.
- Select multiple remote files and click `Queue download(s)`.
- Select multiple local files and click `Queue upload(s)`.
- Queue table shows queued items.
- `Start queue` processes items sequentially.
- Done items appear in the expected local/remote folder.
- `Remove selected` removes selected pending/non-running queue items.
- `Clear finished` removes Done/Failed/Cancelled/Skipped rows.
- Folder selections are skipped/refused.
- Terminal, paste, Ctrl+C, and terminal Session Traffic still work.

## Not expected yet

- Folder upload/download.
- Parallel transfers.
- Resume.
- Drag/drop.
- Sync engine.
- Delete/rename/chmod/mkdir.
- SFTP bytes in the live traffic monitor.
- Overwrite metadata comparison.
