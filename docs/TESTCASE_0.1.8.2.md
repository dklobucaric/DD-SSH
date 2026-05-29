# Testcase — dev 0.1.8.2

Checkpoint: `dev 0.1.8.2` — File manager safety polish.

## Basic validation

- [ ] About dialog shows `dev 0.1.8.2`.
- [ ] Terminal session opens normally.
- [ ] Paste and Ctrl+C still work in terminal.
- [ ] File Manager opens normally.
- [ ] Local panel shows `Queue upload`.
- [ ] Remote panel shows `Queue download`.

## Queue regression

- [ ] Single-file download still works.
- [ ] Single-file upload still works.
- [ ] Multiple-file queue still works.
- [ ] Overwrite / Skip / Overwrite all / Skip all still work.
- [ ] Retry selected still returns Done/Failed/Cancelled/Skipped items to Pending.
- [ ] Start queue with no Pending items shows a clear message.

## Folder safety polish

- [ ] Queue a small local folder upload.
- [ ] Folder confirmation includes experimental/safety text.
- [ ] Folder scan summary appears after scan.
- [ ] Summary counts folder files and folder create items.
- [ ] Queue starts only after the user clicks Start queue.
- [ ] Queue completes and refreshes the remote panel.
- [ ] Queue a small remote folder download.
- [ ] Folder confirmation includes experimental/safety text.
- [ ] Folder scan summary appears after scan.
- [ ] Queue completes and refreshes the local panel.

## Edge checks

- [ ] Folder with a symlink or special file reports skipped item(s), if available for testing.
- [ ] Permission/path failure shows a useful queue item message with a path.
- [ ] Existing overwrite behavior still works inside folder-expanded queue items.
