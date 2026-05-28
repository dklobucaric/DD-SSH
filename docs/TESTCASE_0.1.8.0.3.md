# Testcase — dev 0.1.8.0.3

Checkpoint: `dev 0.1.8.0.3` — Transfer queue retry-selected polish.

## Basic validation

- [ ] About dialog shows `dev 0.1.8.0.3`
- [ ] File Manager opens from a saved session
- [ ] Local panel lists local files
- [ ] Remote panel lists remote SFTP files
- [ ] Queue table shows Status / Direction / Name / Size / Source / Target
- [ ] `Retry selected` button is visible between `Start queue` and `Remove selected`

## Retry completed downloads

- [ ] Queue two or more remote files for download
- [ ] Start queue and let them finish as `Done`
- [ ] Select one `Done` item
- [ ] Click `Retry selected`
- [ ] Selected item changes back to `Pending`
- [ ] Click `Start queue`
- [ ] Existing local overwrite prompt appears when the file already exists
- [ ] `Overwrite` runs only that item
- [ ] `Skip` marks that retried item as `Skipped`

## Retry completed uploads

- [ ] Queue two or more local files for upload
- [ ] Start queue and let them finish as `Done`
- [ ] Select one `Done` upload item
- [ ] Click `Retry selected`
- [ ] Selected item changes back to `Pending`
- [ ] Click `Start queue`
- [ ] Existing remote overwrite prompt appears when the file already exists
- [ ] `Overwrite` runs only that item
- [ ] `Skip` marks that retried item as `Skipped`

## Retry cancelled/failed/skipped states

- [ ] Cancel a queued download or upload item
- [ ] Select the `Cancelled` item
- [ ] Click `Retry selected`
- [ ] Item changes to `Pending`
- [ ] Start queue can run it again
- [ ] Create or simulate a `Failed` queue item
- [ ] `Retry selected` moves it back to `Pending`
- [ ] Create a `Skipped` overwrite decision
- [ ] `Retry selected` moves it back to `Pending`

## Safety checks

- [ ] Selecting a `Pending` item and clicking `Retry selected` leaves it `Pending`
- [ ] Queue cannot retry a `Running` item while the queue is active
- [ ] `Remove selected` still removes selected non-running queue items
- [ ] `Clear finished` still removes Done/Failed/Cancelled/Skipped queue items
- [ ] Queue remains sequential; no parallel transfer is expected
- [ ] Folder transfer remains blocked/skipped

## Regression checks

- [ ] `Download selected now` still works for one remote file
- [ ] `Upload selected now` still works for one local file
- [ ] Queue overwrite dialog still offers Overwrite / Skip / Overwrite all / Skip all / Cancel queue
- [ ] Terminal xterm.js still connects
- [ ] Paste / Ctrl+C still work
- [ ] Terminal Session Traffic still works for terminal tabs only
