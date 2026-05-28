# Testcase — dev 0.1.8.0

Checkpoint: **Transfer queue foundation**

## Required tests

```text
[ ] Build succeeds on Linux
[ ] About shows dev 0.1.8.0
[ ] Open File Manager from a saved session
[ ] Local panel lists local files
[ ] Remote panel lists remote SFTP files
[ ] Download selected now still works for one remote file
[ ] Upload selected now still works for one local file
[ ] Select multiple remote files
[ ] Queue download(s) adds multiple Download rows
[ ] Select multiple local files
[ ] Queue upload(s) adds multiple Upload rows
[ ] Queue table shows Status / Direction / Name / Size / Source / Target
[ ] Start queue processes items one by one
[ ] Download queue item becomes Done and file appears locally
[ ] Upload queue item becomes Done and file appears remotely
[ ] Existing local overwrite prompt appears for queued downloads when target exists
[ ] Existing remote overwrite prompt/skip behavior appears for queued uploads when target exists
[ ] Cancelling queued download reports that local target was not replaced
[ ] Cancelling queued upload reports that partial remote file may remain
[ ] Remove selected removes selected non-running queue items
[ ] Clear finished removes Done/Failed/Cancelled/Skipped items
[ ] Folder upload/download remains blocked or skipped
[ ] Terminal xterm.js still connects
[ ] Paste / Ctrl+C still work
[ ] Terminal Session Traffic still works for terminal tabs
```

## Cross-platform smoke

```text
[ ] Linux build/test
[ ] Windows build/test
[ ] macOS build/test
```

## Notes

Queue execution is intentionally sequential. No folder transfer, parallel transfer, resume, sync, drag/drop, or SFTP traffic monitor integration is expected in this checkpoint.
