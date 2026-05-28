# Testcase — dev 0.1.7.7

Checkpoint: **Single-file SFTP upload foundation**

## Required tests

```text
[ ] Build succeeds on Linux
[ ] About shows dev 0.1.7.7
[ ] Open File Manager from a saved session
[ ] Local panel lists local files
[ ] Remote panel lists remote SFTP files
[ ] Download selected still works
[ ] Upload selected uploads one small local file
[ ] Uploaded file appears after remote refresh
[ ] Uploading the same filename asks before overwrite
[ ] Cancel on overwrite does not upload
[ ] Folder upload is rejected
[ ] Large-ish upload smoke test passes if convenient
[ ] Terminal xterm.js still connects
[ ] Paste / Ctrl+C still work
[ ] Terminal Session Traffic still works
```

## Notes

Cancel during upload may leave a partial remote file. Cleanup/cancel polish is intentionally deferred to later transfer-progress polish.
