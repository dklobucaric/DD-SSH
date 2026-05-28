# Testcase — dev 0.1.8.0.2

Checkpoint: **dev 0.1.8.0.2 — Transfer queue overwrite-all polish**

## Goal

Validate that queued transfers with repeated overwrite conflicts can be handled with one decision: `Overwrite all` or `Skip all`.

## Preconditions

- At least one saved session with SFTP enabled.
- File Manager opens successfully.
- Have multiple remote files whose names already exist in the selected local folder.
- Have multiple local files whose names already exist in the selected remote folder.

## Download Overwrite all test

1. Queue at least three remote files that already exist in the current local folder.
2. Click **Start queue**.
3. On the first overwrite prompt, choose **Overwrite all**.
4. Expected: remaining existing local targets are overwritten without asking again.
5. Expected: queue summary reports the completed items as Done.

## Download Skip all test

1. Queue at least three remote files that already exist in the current local folder.
2. Click **Start queue**.
3. On the first overwrite prompt, choose **Skip all**.
4. Expected: remaining existing local targets are marked Skipped without asking again.

## Upload Overwrite all test

1. Queue at least three local files that already exist in the current remote folder.
2. Click **Start queue**.
3. On the first remote overwrite prompt, choose **Overwrite all**.
4. Expected: remaining existing remote targets are overwritten without asking again.

## Upload Skip all test

1. Queue at least three local files that already exist in the current remote folder.
2. Click **Start queue**.
3. On the first remote overwrite prompt, choose **Skip all**.
4. Expected: remaining existing remote targets are marked Skipped without asking again.

## Regression checks

- `Overwrite` affects only the current queue item.
- `Skip` affects only the current queue item.
- `Cancel queue` stops the queue cleanly.
- Immediate Download selected now still works.
- Immediate Upload selected now still works.
- Terminal tabs still work.
- Paste / Ctrl+C terminal behavior still works.
