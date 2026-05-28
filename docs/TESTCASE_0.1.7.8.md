# Testcase — dev 0.1.7.8

Checkpoint: **Transfer progress and cancel polish**

## Required tests

```text
[ ] Build succeeds on Linux
[ ] About shows dev 0.1.7.8
[ ] Open File Manager from a saved session
[ ] Local panel lists local files
[ ] Remote panel lists remote SFTP files
[ ] Download selected still works for a small file
[ ] Download progress dialog shows transferred size / total size
[ ] Download progress dialog shows speed
[ ] Download progress dialog shows elapsed time
[ ] Download completion dialog shows formatted size and raw bytes
[ ] Download completion dialog shows elapsed time and average speed
[ ] Cancel download shows a clear cancellation dialog
[ ] Cancel download does not replace the existing local target file
[ ] Upload selected still works for a small file
[ ] Upload progress dialog shows transferred size / total size
[ ] Upload progress dialog shows speed
[ ] Upload progress dialog shows elapsed time
[ ] Upload completion dialog shows formatted size and raw bytes
[ ] Upload completion dialog shows elapsed time and average speed
[ ] Cancel upload shows a clear cancellation dialog
[ ] Cancel upload warns that a partial remote file may remain
[ ] Existing overwrite prompts still work
[ ] Folder upload/download are rejected clearly
[ ] Terminal xterm.js still connects
[ ] Paste / Ctrl+C still work
[ ] Terminal Session Traffic still works
```

## Cross-platform smoke

```text
[ ] Linux build/test
[ ] Windows build/test
[ ] macOS build/test
```

## Notes

Download uses `QSaveFile`, so cancelling a download should not replace the local target file. Upload cancellation can leave a partial remote file because the remote file is written directly through the SFTP subsystem. Transfer queue, folder transfer, and SFTP Session Traffic integration remain later checkpoints.
