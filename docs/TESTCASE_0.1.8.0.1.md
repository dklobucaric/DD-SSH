# Testcase — dev 0.1.8.0.1

Checkpoint: **dev 0.1.8.0.1 — Transfer queue overwrite prompt polish**

## Goal

Validate that transfer queue overwrite prompts are visible and not blocked by the queue progress dialog.

## Preconditions

- At least one saved session with SFTP enabled.
- File Manager opens successfully.
- Have a local test file whose name already exists in the selected remote folder.
- Have a remote test file whose name already exists in the selected local folder.

## Upload overwrite test

1. Select a local file that already exists in the current remote folder.
2. Click **Queue upload(s)**.
3. Click **Start queue**.
4. Expected: overwrite prompt is visible on top, not hidden under the queue progress dialog.
5. Choose **No**.
6. Expected: item becomes **Skipped** and queue continues.
7. Queue the same file again.
8. Choose **Yes**.
9. Expected: upload runs with progress and completes.
10. Queue the same file again.
11. Choose **Cancel**.
12. Expected: item becomes **Cancelled** and queue stops cleanly.

## Download overwrite test

1. Select a remote file that already exists in the current local folder.
2. Click **Queue download(s)**.
3. Click **Start queue**.
4. Expected: overwrite prompt appears before progress starts.
5. Choose **No**.
6. Expected: item becomes **Skipped** and queue continues.
7. Repeat and choose **Yes**.
8. Expected: download runs with progress and completes.
9. Repeat and choose **Cancel**.
10. Expected: item becomes **Cancelled** and queue stops cleanly.

## Regression checks

- Immediate Download selected now still works.
- Immediate Upload selected now still works.
- Queue still processes items sequentially.
- Progress still shows transferred size, elapsed time, and average speed.
- Terminal tabs still work.
- Paste / Ctrl+C terminal behavior still works.
